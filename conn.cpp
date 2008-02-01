

#include "request.h" 
#include "conn.h"
#include <glib.h>
#include <errno.h>
#include <debug.h>

using namespace rbol ;
using namespace std; 

#include <connection.h>

std::set<PurpleAsyncConn*> PurpleAsyncConn::valid_PurpleAsyncConns; 

bool PurpleAsyncConn::isInvalid() {
	return valid_PurpleAsyncConns.count(this) == 0 ;
}
bool PurpleAsyncConn::setInvalid() {
	return	valid_PurpleAsyncConns.erase(this);
}


static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond);

PurpleAsyncConn::PurpleAsyncConn(PurpleAsyncConnHandler *_handler,
				 int pm ) :awaiting("") 
{ 
	valid_PurpleAsyncConns.insert(this) ;
	handler = _handler ;
	txbuf = purple_circ_buffer_new(0);
	parse_mode = pm ;
	ref_counter = 1 ; 
	rx_handler = NULL ;
	tx_handler = NULL ;
}

PurpleAsyncConn::~PurpleAsyncConn() { 
	close() ;
}

void 
PurpleAsyncConn::got_connected_cb(gint source, const gchar* error) { 

	if ( isInvalid() ) { 
		purple_debug_info("rbol", "oops, connection is no longer valid!\n");
		return ;
	}


	if ( source < 0 ) { 
		handler->connectionError(error) ;
		close() ;

		setInvalid() ;
		purple_debug_error( "rbol", "connection error '%s'\n", 
			     error) ;
		return ;
	}

	purple_debug(PURPLE_DEBUG_INFO, "rbol", "got connected\n") ;
	fd = source ; 
	
	/* so now this callback need not take care of connection issues */
	rx_handler = purple_input_add(fd, PURPLE_INPUT_READ, 
					  conn_read_cb, this) ;
	handler->gotConnected() ;
}



static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message) ;

bool 
PurpleAsyncConn::establish_connection(
	const string ip, 
	const int port) {
 
	tx_handler = NULL ; 
	rx_handler = NULL ;	
	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "establishing connection\n");
	fd = -1 ;
	
	if ( purple_proxy_connect(NULL, handler->getProxyAccount(), 
				  ip.c_str(), 
				  port, conn_got_connected, this) ==NULL) {
		purple_debug(PURPLE_DEBUG_ERROR, "rbol", 
			     "connection failed\n") ;
		return false ;
	}
	return true ;
}


static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message) { 

	PurpleAsyncConn *conn = (PurpleAsyncConn*)data; 
	conn->got_connected_cb(source, error_message) ;

}



/* shutdowns the connection, waits till all packets are sent. 
 * TODO: fix  but returns immediately */
void 
PurpleAsyncConn::close() { 
	setInvalid() ;
	purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
		     "In here\n") ;
	if ( txbuf ) {
		purple_circ_buffer_destroy(txbuf) ;
		txbuf = NULL; 
	}
	if ( tx_handler ) { 
		purple_input_remove(tx_handler) ;
		tx_handler = NULL ;
	}
	if ( rx_handler ) { 
		purple_input_remove(rx_handler) ;
		rx_handler = NULL ;
	}
	handler = NULL ;
	
	
	if (fd > 0 ) {
		::close(fd) ;
		fd = 0 ; 
	}
	
}

static void conn_write_cb( gpointer data, gint source, 
			   PurpleInputCondition cond) ;



void  
PurpleAsyncConn::write(const void* data, 
	int datalen) { 
	int written ; 


	if ( !tx_handler) 
		written = ::write(fd, data, datalen) ;
	else { 
		written = -1 ;
		errno = EAGAIN ; 
	}

	if ( written < 0 && errno == EAGAIN ) 
		written = 0 ; 
	else if ( written <= 0 ) { 
		written = 0 ; 
		handler->readError() ;
		close() ;
		return ;
	}

	if ( written < datalen ) { 
		purple_circ_buffer_append(txbuf, ((char*)data)+written,
					  datalen-written) ;

		if ( ! tx_handler) 
			tx_handler = purple_input_add(fd, 
				  PURPLE_INPUT_WRITE, 
				  conn_write_cb, 
				  this);


	}

	handler->writeCallback() ;
}


void 
PurpleAsyncConn::write_cb() { 
	int writelen , ret; 
	
	writelen = purple_circ_buffer_get_max_read(txbuf) ; 
	if ( writelen == 0 ) { 
		purple_input_remove(tx_handler) ;
		tx_handler = 0 ;
		return ;
	}

	ret = ::write(fd, txbuf->outptr, writelen) ;

	if ( ret < 0 && errno == EAGAIN ) {
		return ; 
	}
	else if ( ret <= 0 ) { 
		return ; 
	}

	purple_circ_buffer_mark_read(txbuf, ret) ;


}


static void conn_write_cb( gpointer data, gint source, 
			   PurpleInputCondition cond) { 
	PurpleAsyncConn* conn = (PurpleAsyncConn*) data ; 

	conn->write_cb() ; 
}

#include <iostream>
void
PurpleAsyncConn::read_cb() { 

	if ( isInvalid() ) { 
		purple_debug_warning("rbol", "Technically, should not get a readcallback on invalid connectin\n") ;
		return ;
	}

	char buf[1024] ;
	int len = read(fd, buf, sizeof(buf)) ;
	if( len < 0 ) {
		if ( errno == EAGAIN )
			return ; /* safe */
		
		/* todo: register a connection error */
		handler->readError() ;
		close();
		cerr<<"out here\n" ;
		return ;
	} else if ( len == 0 ) { 
		/* todo: server closed conenction */ 
		
		handler->closeCallback();
		close() ;

		purple_debug(PURPLE_DEBUG_ERROR, "rbol",
			     "pathetic, the server has closed the connection\n");
		return ;
	}
	
	awaiting.push(string(buf, buf+len)) ;

	handler->readCallback(awaiting);
}

static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond) {
	((PurpleAsyncConn*)data)->read_cb() ;
}

void PurpleAsyncConn::write(string s ) { 
	write(s.data(), s.length());
}
