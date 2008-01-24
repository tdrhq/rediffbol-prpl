

#include "request.h" 
#include "conn.h"
#include <glib.h>
#include <errno.h>

using namespace rbol ;
using namespace std; 

typedef bool(*PacketRecvCallback)(struct RediffBolConn*, struct Request *) ; 
typedef bool(*GotConnectedCallback)(PurpleAccount*) ;

static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond);

PurpleAsyncConn::PurpleAsyncConn(RediffBolConn *conn, string ip, gint32 port,
	int pm ) 
:awaiting("") 
{ 
	rb_conn = conn ;
	establish_connection(ip, port) ;
	txbuf = purple_circ_buffer_new(0);
	parsemode = pm ;
	ref_counter = 1 ; 
}

PurpleAsyncConn::~PurpleAsyncConn() { 
	purple_circ_buffer_destroy(txbuf) ;
	close() ;
}

void 
PurpleAsyncConn::got_connected_cb(gint source) { 

	if ( source < 0 ) { 
		/* todo */
	}

	fd = source ; 
	
	/* so now this callback need not take care of connection issues */
	rx_handler = purple_input_add(fd, PURPLE_INPUT_READ, 
					  conn_read_cb, this) ;
	rb_conn->got_connected_cb() ;
}



static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message) ;

bool 
PurpleAsyncConn::establish_connection(
	const string ip, 
	const int port
	) { 
	
	fd = -1 ;
	
	if ( purple_proxy_connect(NULL, rb_conn->account, 
				  ip.c_str(), 
				  port, conn_got_connected, this) ==NULL) {
		return false ;
	}
	
	tx_handler = NULL ; 
	rx_handler = NULL ;
}


static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message) { 

	PurpleAsyncConn *conn = (PurpleAsyncConn*)data; 
	RediffBolConn *rb = conn->rb_conn; 

	conn->got_connected_cb(source) ;

}



/* shutdowns the connection, waits till all packets are sent. 
 /* TODO: fix  but returns immediately */
bool 
PurpleAsyncConn::close() { 
	
	if (fd > 0 ) 
		::close(fd) ;
	
	purple_circ_buffer_destroy(txbuf) ;
}

static void conn_write_cb( gpointer data, gint source, 
			   PurpleInputCondition cond) ;



void  
PurpleAsyncConn::write(void* data, 
	int datalen) { 
	RediffBolConn* rb = rb_conn ;
	int written ; 

	if ( !tx_handler) 
		written = ::write(rb->fd, data, datalen) ;
	else { 
		written = -1 ;
		errno = EAGAIN ; 
	}

	if ( written < 0 && errno == EAGAIN ) 
		written = 0 ; 
	else if ( written <= 0 ) { 
		written = 0 ; 
	}

	if ( written < datalen ) { 

		if ( ! tx_handler) 
			tx_handler = purple_input_add(fd, 
				  PURPLE_INPUT_WRITE, 
				  conn_write_cb, 
				  this);

		purple_circ_buffer_append(txbuf, ((char*)data)+written,
					  datalen-written) ;

	}
}


void 
PurpleAsyncConn::write_cb() { 
	int writelen , ret; 
	
	writelen = purple_circ_buffer_get_max_read(txbuf) ; 
	if ( writelen == 0 ) { 
		purple_input_remove(tx_handler) ;
		tx_handler = 0 ;
		if ( ref_counter == 0 ) 
			delete this ;
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
	RediffBolConn * rb = conn->rb_conn  ;

	conn->write_cb() ; 
}

void
PurpleAsyncConn::read_cb() { 
	char buf[1024] ;
	int len = read(fd, buf, sizeof(buf)) ;
	if( len < 0 ) {
		if ( errno == EAGAIN )
			return ; /* safe */
		
		/* todo: register a connection error */
		return ;
	} else if ( len == 0 ) { 
		/* todo: server closed conenction */ 
	}
	
	awaiting.push(string(buf, len)) ;

	Response* resp = parseResponse(awaiting) ; 
	if ( resp != NULL )  rb_conn->executeResponse(resp) ;
}

static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond) {
	
}
