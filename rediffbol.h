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
#include <set>
typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);
namespace rbol { 
	
	static const char* CAP_DIR = "RBOL/1.2.5" ;
        static const char* CAP_HTTP_CONNECT = "RBOL/1.2.5+HTTP_CONNECT" ;
        static const char*  CAP_HTTP = "RBOL/1.2.5+HTTP" ;
//	static const char* DEFAULT_USERAGENT = "Rediff Bol8.0 build 315" ;
	static const char* DEFAULT_USERAGENT = "libpurple:rediffbol-prpl"; 
	extern void hex_dump (const std::string a,const std::string message) ;
	
	class PurpleAsyncConn ;

	class RediffBolConn : public PurpleAsyncConnHandler { 
	private:

		static std::set<RediffBolConn* > valid_connections ;

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

		bool isInvalid() ;
		void setInvalid() ; 


		void softDestroy() ;

		std::map<std::string, std::string> nickname; 
		std::map<std::string, std::string> status_text ;
		std::map<std::string, std::string> group; 

		std::string _parseChatMessage(MessageBuffer &buffer) ;
		
	public: 
		PurpleAccount *account ;
		PurpleAsyncConn *connection ; 
		std::string userAgent ;

		RediffBolConn(PurpleAccount *acct) ; 

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
		virtual void readError() ;
		void parseGkResponse(MessageBuffer &buffer) ;
		void parseCSResponse(MessageBuffer &buffer) ;
		void parseNewMailsResponse(MessageBuffer &buffer) ;
		void parseContactAddRequest(MessageBuffer &buffer );
		void sendDelContactRequest(std::string, std::string) ;
		void sendGetAddRequest() ;
		void sendAddContactRequest(std::string remoteid, 
					   std::string group) ;
		void setStatus(std::string status, std::string message) ;
		void startLogin() ;
		std::string fixEmail(std::string a) ;
		std::string getBuddyNickname(std::string buddyname) ; 
		std::string getBuddyStatusMessage(std::string buddyname) ;
		virtual void closeCallback() ;
		void sendAcceptAddRequest(std::string localid, 
					  std::string reqId, 
					  std::string from, std::string from2,
					  std::string group);
		void sendDenyAddRequest(std::string localid, 
					  std::string reqId, 
					  std::string from, 
					std::string from2,
					  std::string group);
		void parseOfflineAddContactResponse(MessageBuffer &buffer) ;
		void parseGetContactIdResponse(MessageBuffer &buffer);
		void parseMessageFromMobileUser(MessageBuffer &buffer);
		
		void sendTypingNotification(std::string user) ;
		void parseTypingNoficiationResponse(MessageBuffer &buffer);

	};
	
}

#endif
