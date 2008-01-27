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
#include <vector>
typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);
namespace rbol { 
	
	static const char* CAP_DIR                      = "RBOL/1.2.5" ;
        static const char* CAP_HTTP_CONNECT = "RBOL/1.2.5+HTTP_CONNECT" 
;
        static const char*  CAP_HTTP             = "RBOL/1.2.5+HTTP" ;
	static const char* DEFAULT_USERAGENT = "Rediff Bol8.0 build 315" ;

	class PurpleAsyncConn ;

	class RediffBolConn { 
	private:
		
		/* Response Parsers */

		/* Login Responses */
		void parseCSLoginResponse(MessageBuffer buffer) ;

		/* Responses affecting contacts */
		void parseCSContacts(MessageBuffer &buffer) ;
		void parseContactStatusChange(MessageBuffer &buffer);

		/* basic message handling */
		void parseTextMessage(MessageBuffer &buffer) ;
		void parseOfflineMessages(MessageBuffer &buffer) ;

		std::vector<std::string> roster ; 
		std::string session ;
		int keep_alive_counter ; 
	public: 
		int fd ; 
		PurpleAccount *account ;
		PurpleAsyncConn *connection ; 
		std::string userAgent ;

		RediffBolConn(PurpleAccount *acct) { 
			purple_debug(PURPLE_DEBUG_INFO, "rbol" , "creating a connection\n");

			account  = acct; 
			account -> gc -> proto_data = this ;
			connection = NULL ;
			userAgent = DEFAULT_USERAGENT ;
			keep_alive_counter = 0 ;
		}

		std::list<std::string> iplist ; 


		PurpleAsyncConn* getAsyncConnection() ; 

		void connectToGK() ;
		void connectToCS() ;
		void got_connected_cb() ;

		void sendMessage(std::string to, std::string message) ;
		void sendKeepAlive() ;
		void sendOfflineMessagesRequest() ;

		/**
		 * there was an error in the network. This should also
		 * set the state to offline and/or reconnect if required.
		 */
		void setStateNetworkError(std::string msg) ;

		void parseResponse(MessageBuffer &buffer) ;
		void parseGkResponse(MessageBuffer &buffer) ;
		void parseCSResponse(MessageBuffer &buffer) ;

		void startLogin() ;
	};
	
}

#endif
