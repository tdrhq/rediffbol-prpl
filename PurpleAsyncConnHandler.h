#ifndef __purple_async_conn_handler__h__
#define __purple_async_conn_handler__h__

#include "conn.h"
#include <account.h>
#include "RObject.h"

namespace rbol { 

	class PurpleAsyncConnHandler : public RObject { 
	private:
		virtual void gotConnected()  = 0 ; 
		virtual void readCallback(MessageBuffer &buffer) = 0 ; 
		

		/* this is called before the actual buffer */
		virtual void writeCallback() { 
		}
		
		/* failed to connect */
		virtual void connectionError(std::string message) = 0 ;

		/* there was a network error in reading data*/
		virtual void readError() = 0 ;
		
		/* the server closed the connection */
		virtual void closeCallback() = 0 ;
		
		virtual PurpleAccount* getProxyAccount() = 0 ; 

		friend class PurpleAsyncConn ;

	public:
		virtual ~PurpleAsyncConnHandler() { } ;
		
	} ;
}

#endif
