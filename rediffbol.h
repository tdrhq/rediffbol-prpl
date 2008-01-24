#ifndef __REDIFFBOL_H__
#define __REDIFFBOL_H__ 

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glib.h>

#include "account.h"
#include "accountopt.h"
#include "blist.h"
#include "debug.h"
#include "notify.h"
#include "privacy.h"
#include "prpl.h"
#include "roomlist.h"
#include "status.h"
#include "util.h"
#include "version.h"
#include <curl/curl.h>
#include <assert.h>
#include "xmlnode.h"
#include <ctype.h>


#include "request.h"
#include "conn.h"

typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);
namespace rbol { 
	
	static const char* CAP_DIR                      = "RBOL/1.2.5" ;
        static const char* CAP_HTTP_CONNECT = "RBOL/1.2.5+HTTP_CONNECT" 
;
        static const char*  CAP_HTTP             = "RBOL/1.2.5+HTTP" ;
	static const char* DEFAULT_USERAGENT = "Rediff Bol8.0 build 315" ;
	
	class RediffBolConn { 
	public: 
		int fd ; 
		PurpleAccount *account ;
		PurpleAsyncConn *connection ; 
		string userAgent ;

		RediffBolConn(PurpleAccount *acct) { 
			account  = acct; 
			connection = NULL ;
			userAgent = DEFAULT_USERAGENT ;
		}

		std::list<std::string> iplist ; 


		PurpleAsyncConn* getAsyncConnection() ; 

		void connectToGK() ;
		void connectToCS() ;
		void got_connected_cb() ;

		/**
		 * there was an error in the network. This should also
		 * set the state to offline and/or reconnect if required.
		 */
		void setStateNetworkError(string msg) ;

		bool parseResponse(MessageBuffer &buffer, int requesttype) ;
		bool parseGkResponse(MessageBuffer &buffer) ;
		bool parseCSResponse(MessageBuffer &buffer) ;

		void startLogin() ;
	};
	
}

#endif
