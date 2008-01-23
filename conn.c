

#include "request.h" 

typedef bool(*PacketRecvCallback)(struct Request *) ; 

bool conn_establish_connection(
	PurpleAccount* acct, 
	const gchar* ip, 
	const int port,
	PacketRecvCallback rx_cb) { 
	
	RediffBolConn * rb = acct->gc->proto_data ; 
	
	rb -> rx_cb = rx_cb ;

	rb->fd = -1 ;
	
	if ( purple_proxy_connect(NULL, acct, 
				  ip, port, conn_got_connected, acct) ==NULL) {
		return false ;
	}
}

