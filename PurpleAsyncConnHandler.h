#ifndef __purple_async_conn_handler__h__
#define __purple_async_conn_handler__h__

#include "conn.h"
#include <account.h>

namespace rbol { 

	class PurpleAsyncConnHandler { 
	private:
		virtual void gotConnected()  = 0 ; 
		virtual void readCallback(MessageBuffer &buffer) = 0 ; 
		

		/* this is called before the actual buffer */
		virtual void writeCallback() { 
		}
		
		/* failed to connect */
		virtual void connectionError(std::string message) { 
		}

		/* there was a network error in reading data*/
		virtual void readError()  {
		}
		
		/* the server closed the connection */
		virtual void closeCallback() {
		}
		
		virtual PurpleAccount* getProxyAccount() = 0 ; 

		friend class PurpleAsyncConn ;

		
	} ;
}

#endif
