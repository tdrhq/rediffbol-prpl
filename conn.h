#ifndef __CONN_H__
#define __CONN_H__

#include <list>
#include <glib.h>
#include <libpurple/circbuffer.h>
#include "messagebuffer.h"
#include <string>
#include "rediffbol.h"
#include "response.h"

namespace rbol {
	class PurpleAsyncConn { 
	private:
		PurpleCircBuffer *txbuf ;
		MessageBuffer awaiting; 
		guint tx_handler; 
		guint rx_handler; 

		int fd ;
	public:
		RediffBolConn *rb_conn ;
		PurpleAsyncConn(RediffBolConn* conn, 
				std::string ip, 
				gint32 port) ;
		bool establish_connection(
			std::string ip, 
			gint32 port) ;

		~PurpleAsyncConn() ;
		
		void write(gpointer data, int len) ; 
		void write(const Response* resp) ;

		/* internal callbacks */
		void write_cb() ;
		void read_cb() ;

		bool close() ;
		void got_connected_cb(int source) ;
		/* static callbacks */
		static void conn_write_cb( gpointer data, gint source, 
					   PurpleInputCondition cond) ;
		static void conn_read_cb( gpointer data, gint source, 
					  PurpleInputCondition cond) ;

		
	};
}

#endif
