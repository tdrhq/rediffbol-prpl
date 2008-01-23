

#include "request.h" 

typedef bool(*PacketRecvCallback)(struct RediffBolConn*, struct Request *) ; 
typedef bool(*GotConnectedCallback)(PurpleAccount*) ;

bool conn_establish_connection(
	PurpleAccount* acct, 
	const gchar* ip, 
	const int port,
	PacketRecvCallback rx_cb,
	) { 
	
	RediffBolConn * rb = acct->gc->proto_data ; 
	
	rb -> rx_cb = rx_cb ;

	rb->fd = -1 ;
	
	if ( purple_proxy_connect(NULL, acct, 
				  ip, port, conn_got_connected, acct) ==NULL) {
		return false ;
	}
	rb-> txbuf = purple_circ_buffer_init(0) ;
	rb->tx_handler = NULL ; 
}

static void conn_got_connected(gpointer data, gint source, 
			       const gchar * error_message) { 

	PurpleAccount *account = data; 
	PurpleConnection * gc  = account->gc; 
	RediffBolConn *rb = gc->proto_data; 

	if ( source < 0 ) { 
		/* todo */
	}

	rb->fd = source ; 
	
	/* so now this callback need not take care of connection issues */
	rb->got_connected_cb(account) ;
	
}

/* shutdowns the connection, waits till all packets are sent. 
 /* TODO: fix*/  but returns immediately */
static bool conn_close(PurpleAccount *account) { 
	/* who cares .. just dump the connection for the time being */
	
	RediffBolConn* rb = account->gc->proto_data ;
	
	if (rb->fd > 0 ) 
		close(rb->fd) ;
	
	purple_circ_buffer_destroy(rb->txbuf) ;
}


static bool conn_write(PurpleAccount *account, const gchar* data, 
	int datalen) { 
	RediffBolConn* rb = account->gc->proto_data ;	
	int written ; 

	if ( !rb->tx_handler) 
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

		if ( ! rb->tx_handler) 
			rb->tx_handler = purple_input_add(rb->fd, 
				  PURPLE_INPUT_WRITE, 
				  conn_write_cb, 
				  account);

		purple_circ_buffer_append(rb->txbuf, data+written,
					  len-written) ;

	}
}


static void conn_write_cb( gpointer data, gint source, 
			   PurpleInputCondition cond) { 
	PurpleAccount* account = data;
	RediffBolConn * rb = account->gc->proto_data ;

	int writelen , ret; 
	
	writelen = purple_circ_buffer_get_max_read(rb->txbuf) ; 
	if ( writelen == 0 ) { 
		purple_input_remove(rb->tx_handler) ;
		rb->tx_handler = 0 ;
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
