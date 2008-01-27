#ifndef __CONN_H__
#define __CONN_H__

#include <list>
#include <glib.h>
#include <libpurple/circbuffer.h>
#include "messagebuffer.h"
#include <string>
#include "PurpleAsyncConnHandler.h" 

namespace rbol {
	class PurpleAsyncConn { 
	private:

		/* buffers */
		PurpleCircBuffer *txbuf ;
		MessageBuffer awaiting; 

		/* handlers */
		guint tx_handler; 
		guint rx_handler; 
		
		/* some deprecated counter */
		int ref_counter ;

		/* the purple connection socket */
		int fd ;

		/* store some information for the parent */
		int parse_mode ; 

		/* super handler */
		PurpleAsyncConnHandler *handler ;

	public:
		int getParseMode() { 
			return parse_mode ; 
		}

		PurpleAsyncConn(PurpleAsyncConnHandler* conn, 
				std::string ip, gint32 port, int pm) ;
		bool establish_connection(std::string ip, gint32 port) ;

		~PurpleAsyncConn() ;
		
		void write(const void* data, int len) ; 

		/* internal callbacks */
		void write_cb() ;
		void read_cb() ;

		bool close() ;
		void got_connected_cb(int source, const gchar* ) ;
		/* static callbacks */
//		static void conn_write_cb( gpointer data, gint source, 
		//				   PurpleInputCondition cond) ;
		//static void conn_read_cb( gpointer data, gint source, 
		//		  PurpleInputCondition cond) ;

		void unref() { 
			ref_counter -- ;
		}
		
	};
}

#endif
