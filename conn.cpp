

#include "request.h" 

typedef bool(*PacketRecvCallback)(struct RediffBolConn*, struct Request *) ; 
typedef bool(*GotConnectedCallback)(PurpleAccount*) ;

static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond);
PurpleAsyncConn::PurpleAsyncConn(RediffBolConn *conn, string ip, gint32 port ) { 
	rb_conn = conn ;
	establish_connection(ip, port) ;
}
bool 
PurpleAsyncConn::establish_connection(
	const string ip, 
	const int port,
	) { 
	
	fd = -1 ;
	
	if ( purple_proxy_connect(NULL, acct, 
				  ip, port, conn_got_connected, this) ==NULL) {
		return false ;
	}
	txbuf = purple_circ_buffer_init(0) ;
	
	tx_handler = NULL ; 
	rx_handler = NULL ;
}

static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message) { 

	PurpleAsyncConn *conn = data; 
	PurpleConnection * gc  = account->gc; 
	RediffBolConn *rb = gc->proto_data; 

	if ( source < 0 ) { 
		/* todo */
	}

	fd = source ; 
	
	/* so now this callback need not take care of connection issues */
	rb_conn->got_connected_cb(account) ;
	

	conn->rx_handler = purple_input_add(rb->fd, PURPLE_INPUT_READ, 
					  conn_read_cb, this) ;
}

/* shutdowns the connection, waits till all packets are sent. 
 /* TODO: fix  but returns immediately */
static bool 
PurpleAsyncConn::close() { 
	
	if (fd > 0 ) 
		close(fd) ;
	
	purple_circ_buffer_destroy(rb->txbuf) ;
}

static void conn_write_cb( gpointer data, gint source, 
			   PurpleInputCondition cond) ;

bool 
PurpleAsyncConn::write(const gchar* data, 
	int datalen) { 
	RediffBolConn* rb = rb_conn ;
	int written ; 

	if ( !tx_handler) 
		written = write(rb->fd, data, datalen) ;
	else { 
		written = -1 ;
		errno = EAGAIN ; 
	}

	if ( written < 0 && errno == EAGAIN ) 
		written = 0 ; 
	else if ( written <= 0 ) { 
		written = 0 ; 
	}

	if ( written < len ) { 

		if ( ! tx_handler) 
			tx_handler = purple_input_add(fd, 
				  PURPLE_INPUT_WRITE, 
				  conn_write_cb, 
				  this);

		purple_circ_buffer_append(rb->txbuf, data+written,
					  len-written) ;

	}
}


static void conn_write_cb( gpointer data, gint source, 
			   PurpleInputCondition cond) { 
	PurpleAccount* account = data;
	RediffBolConn * rb = account->gc->proto_data ;

	int writelen , ret; 
	
	writelen = purple_circ_buffer_get_max_read(txbuf) ; 
	if ( writelen == 0 ) { 
		purple_input_remove(tx_handler) ;
		tx_handler = 0 ;
		return ;
	}

	ret = write(rb->fd, rb->txbuf->outptr, writelen) ;

	if ( ret < 0 && errno == EGAIN ) {
		return ; 
	}
	else if ( ret <= 0 ) { 
		return ; 
	}

	purple_circ_buffer_mark_read(rb->txbuf, ret) ;

}

void
PurpleAsyncConn::read_cb(gint source) { 
	char buf[1024] ;
	len = read(fd, buf, sizeof(buf)) ;
	if( len < 0 ) {
		if ( errno == EAGAIN )
			return ; /* safe */
		
		/* todo: register a connection error */
		return ;
	} else if ( len == 0 ) { 
		/* todo: server closed conenction */ 
	}
	
	awaiting.push(string(buf, len)) ;

	/* todo: response parser */
}

static void conn_read_cb(gpointer data, gint source, PurpleInputCondition cond) {
	
}
