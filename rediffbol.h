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
#include <connection.h>
#include <map>

#include "request.h"
#include "conn.h"
#include <vector>
#include "PurpleAsyncConnHandler.h"

typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);
namespace rbol { 
	
	static const char* CAP_DIR = "RBOL/1.2.5" ;
        static const char* CAP_HTTP_CONNECT = "RBOL/1.2.5+HTTP_CONNECT" ;
        static const char*  CAP_HTTP = "RBOL/1.2.5+HTTP" ;
	static const char* DEFAULT_USERAGENT = "Rediff Bol8.0 build 315" ;
	extern void hex_dump (const std::string a,const std::string message) ;
	
	class PurpleAsyncConn ;

	class RediffBolConn : public PurpleAsyncConnHandler { 
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

		/**
		 * there was an error in the network. This should also
		 * set the state to offline and/or reconnect if required.
		 */
		void setStateNetworkError(int  reason, 
					  std::string msg) ;

		std::vector<std::string> roster ; 
		std::string session ;
		int keep_alive_counter ; 
		guint keep_alive_timer_handle ; 
		int connection_state ; 

		bool _is_invalid ; 
		
		bool isInvalid() { 
			return _is_invalid ; 
		}
		
		void setInvalid() { 
			_is_invalid = true ;
		}

		void softDestroy() ;

		std::map<std::string, std::string> nickname; 
		std::map<std::string, std::string> status_text ;

	public: 
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
			connection_state = 0 ;
			keep_alive_timer_handle = 0 ;
			_is_invalid = false ;
		}

		~RediffBolConn() ; 
		std::list<std::string> iplist ; 


		PurpleAsyncConn* getAsyncConnection() ; 

		void connectToGK() ;
		void connectToCS() ;
		void gotConnected() ;

		virtual PurpleAccount* getProxyAccount() ;
		void sendMessage(std::string to, std::string message) ;
		void sendKeepAlive() ;
		void sendOfflineMessagesRequest() ;
		void deleteOfflineMessage(std::string id) ;

		void readCallback(MessageBuffer &buffer) ;
		void parseGkResponse(MessageBuffer &buffer) ;
		void parseCSResponse(MessageBuffer &buffer) ;
		void parseNewMailsResponse(MessageBuffer &buffer) ;
		void sendGetAddRequest() ;
		void setStatus(std::string status, std::string message) ;
		void startLogin() ;

		std::string getBuddyNickname(std::string buddyname) ; 
		std::string getBuddyStatusMessage(std::string buddyname) ;
		virtual void closeCallback() ;
	};
	
}

#endif
