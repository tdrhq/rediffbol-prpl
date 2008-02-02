

#include "request.h" 
#include "conn.h"
#include <glib.h>
#include <errno.h>
#include <debug.h>
#include <cassert>
using namespace rbol ;
using namespace std; 

#include <connection.h>

std::set<PurpleAsyncConn*> PurpleAsyncConn::valid_PurpleAsyncConns; 
namespace rbol { 
	void hex_dump(const string, const string);
}

bool PurpleAsyncConn::isInvalid() {
	return valid_PurpleAsyncConns.count(this) == 0 ;
}
void PurpleAsyncConn::setInvalid() {
	valid_PurpleAsyncConns.erase(this);
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
	connection_attempt_data = NULL ;

	purple_debug_info("rbol", "Connection object created: %x\n", this);
	for(typeof(valid_PurpleAsyncConns.begin()) it = 
		    valid_PurpleAsyncConns.begin() ;
	    it!= valid_PurpleAsyncConns.end() ; it++ ){ 
		purple_debug_info("rbol", "Valid Connection %x\n", *it) ;
	}
}

PurpleAsyncConn::~PurpleAsyncConn() { 
	this->close() ;
}

void 
PurpleAsyncConn::got_connected_cb(gint source, const gchar* error) { 

	connection_attempt_data = NULL ;
	purple_debug_info("rbol", "got an fd of %d\n", source) ;
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
 
	assert(rx_handler == 0 and tx_handler == 0 ) ;

	purple_debug(PURPLE_DEBUG_INFO, "rbol" , "establishing connection\n");
	fd = -1 ;
	
	connection_attempt_data = purple_proxy_connect(NULL, handler->getProxyAccount(), 
						       ip.c_str(), 
						       port, conn_got_connected, this) ; 
	if (  connection_attempt_data == NULL) {
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

	purple_debug_info("rbol", "Closing connection object %x\n", this);
	setInvalid() ;
	purple_debug(PURPLE_DEBUG_INFO, "rbol" ,
		     "In here\n") ;

	if ( connection_attempt_data ) {
		purple_proxy_connect_cancel(connection_attempt_data ) ;
		connection_attempt_data = NULL ; 
	}

	int _tmp ; 

	if (fd > 0 and purple_input_get_error(fd, &_tmp) == 0 ) {
		if ( ::close(fd) < 0 ) { 
			purple_debug_info("rbol", "Failed to close fd with error %d\n", errno) ;
		}
		fd = 0 ; 
	}

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
	} else 
		purple_debug_warning("rbol", "Why is there no rx_handler?\n");

	handler = NULL ;
	
	
	
}

static void conn_write_cb( gpointer data, gint source, 
			   PurpleInputCondition cond) ;



void  
PurpleAsyncConn::write(const void* data, 
	int datalen) { 
	int written ; 

	assert(!isInvalid()) ;

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
		
		purple_debug_error("rbol", "writing failed, going into bad state\n") ;
		handler->readError() ;
		close() ;
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
		purple_debug_info("rbol", "Technically, should not get a readcallback on invalid connectin\n") ;
		assert(false);
		return ;
	}

	purple_debug_info("rbol", "got a read on %x with fd=%d\n", this, fd);
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
	
	hex_dump(string(buf, buf+len), "this is the last received data" );
	purple_debug_info("rbol", "am I even here?\n");
	awaiting.push(string(buf, buf+len)) ;

	handler->readCallback(awaiting);
}

static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond) {

	purple_debug_info("rbol", "source-fd = %d\n", source) ;
	int _tmp ;
	if ( source < 0 or purple_input_get_error(source, &_tmp) != 0 ) { 
		purple_debug_info("rbol", "Callback called on invalid handle\n");
		return ;

	}
	((PurpleAsyncConn*)data)->read_cb() ;
}

void PurpleAsyncConn::write(string s ) { 
	write(s.data(), s.length());
}
