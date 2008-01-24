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
		
		int ref_counter ;
		int fd ;
		int parse_mode ; 
	public:
		int getParseMode() { 
			return parse_mode ; 
		}
		RediffBolConn *rb_conn ;
		PurpleAsyncConn(RediffBolConn* conn, 
				std::string ip, 
				gint32 port,
			int pm) ;
		bool establish_connection(
			std::string ip, 
			gint32 port) ;

		~PurpleAsyncConn() ;
		
		void write(const void* data, int len) ; 
		void write(const Response* resp) ;

		/* internal callbacks */
		void write_cb() ;
		void read_cb() ;

		bool close() ;
		void got_connected_cb(int source) ;
		/* static callbacks */
//		static void conn_write_cb( gpointer data, gint source, 
		//				   PurpleInputCondition cond) ;
		//static void conn_read_cb( gpointer data, gint source, 
		//		  PurpleInputCondition cond) ;

		void unref() { 
			ref_counter -- ;
			if ( ref_counter == 0 and !tx_handler ) 
				delete this ;
		}
		
	};
}

#endif
