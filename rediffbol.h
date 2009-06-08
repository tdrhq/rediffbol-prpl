/**
 * @file rediffbol.h 
 * 
 * Copyright (C) 2008-2009 Arnold Noronha <arnstein87 AT gmail DOT com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __REDIFFBOL_H__
#define __REDIFFBOL_H__ 

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glib.h>
#include "config.h"
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
#include "RObject.h"

typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);
namespace rbol { 
	
	#define  CAP_DIR  "RBOL/1.2.5" 
        #define  CAP_HTTP_CONNECT  "RBOL/1.2.5+HTTP_CONNECT" 
        #define  CAP_HTTP  "RBOL/1.2.5+HTTP" 
	#define  DEFAULT_USERAGENT  "Rediff Bol8.0 build 315" 

	extern void hex_dump (const std::string a, const std::string message);
	
	class PurpleAsyncConn;

	class RediffBolConn : public PurpleAsyncConnHandler { 
	private:

		/* Response Parsers */

		/* Login Responses */
		void parseCSLoginResponse (MessageBuffer buffer);

		/* Responses affecting contacts */
		void parseCSContacts (MessageBuffer &buffer);
		void parseContactStatusChange (MessageBuffer &buffer);

		/* basic message handling */
		void parseTextMessage (MessageBuffer &buffer);
		void parseOfflineMessages (MessageBuffer &buffer);

		/**
		 * there was an error in the network. This should also
		 * set the state to offline and/or reconnect if required.
		 */
		void setStateNetworkError (int  reason, 
					  std::string msg);

		std::vector<std::string> roster;
		std::string session;
		std::string visibleIP;
		int keep_alive_counter;
		guint keep_alive_timer_handle;
		int connection_state;

		void softDestroy ();

		std::map<std::string, std::string> nickname;
		std::map<std::string, std::string> status_text;
		std::map<std::string, std::string> group;
		std::set<std::string> groups;
		std::string _parseChatMessage(MessageBuffer &buffer);
		
		/* usernames as retrieved from server */
		std::string server_userId;
		std::string server_displayname;
		std::string server_nickname;
	public: 
		std::string getServerUserId() const;
		std::string getServerDisplayName() const;
		std::string getServerNickname() const;
		std::string getSessionString() const;
		std::string getVisibleIP() const;
		PurpleAccount *account;
		PurpleAsyncConn *connection;
		std::string userAgent;

		RediffBolConn (PurpleAccount *acct);

		~RediffBolConn();
		std::list<std::string> iplist;


		PurpleAsyncConn* getAsyncConnection();

		void connectToGK();
		void connectToCS();
		void gotConnected();

		virtual PurpleAccount* getProxyAccount();
		void sendMessage(std::string to, std::string message);
		void sendKeepAlive();
		void sendOfflineMessagesRequest();
		void deleteOfflineMessage(std::string id);

		void readCallback(MessageBuffer &buffer, PurpleAsyncConn*);
		virtual void readError(PurpleAsyncConn*);
		void parseGkResponse(MessageBuffer &buffer);
		void parseCSResponse(MessageBuffer &buffer);
		void parseNewMailsResponse(MessageBuffer &buffer);
		void parseContactAddRequest(MessageBuffer &buffer);
		void sendDelContactRequest(std::string, std::string);
		void sendGetAddRequest();
		void sendAddContactRequest(std::string remoteid, std::string group);
		void sendChangeBuddyGroupRequest(std::string buddy, std::string from_group, std::string to_group);

		void setStatus(std::string status, std::string message);
		void startLogin();
		void startLoginOver80 ();
		static std::string fixEmail(std::string a);
		std::string getBuddyNickname(std::string buddyname);
		std::string getBuddyStatusMessage(std::string buddyname);
		virtual void closeCallback(PurpleAsyncConn*);
		void sendAcceptAddRequest(std::string localid, 
					  std::string reqId, 
					  std::string from, std::string from2,
					  std::string group);
		void sendDenyAddRequest(std::string localid, 
					std::string reqId, 
					std::string from, 
					std::string from2,
					std::string group);
		void parseOfflineAddContactResponse(MessageBuffer &buffer);
		void parseGetContactIdResponse(MessageBuffer &buffer);
		void parseMessageFromMobileUser(MessageBuffer &buffer);
		
		void sendTypingNotification(std::string user);
		void parseTypingNoficiationResponse(MessageBuffer &buffer);
		void sendAddRemoveGroupRequest(std::string groupname, bool Remove = false);

		void loadAvatar(std::string username);
		void _loadAvatarCompleted(std::string name, std::string data);

		void connectionError(std::string error, PurpleAsyncConn* conn);
		bool shutdown() { 
			softDestroy();
			return true;
		}



		/**
		 * The following code is meant for chat room purposes
		 * only.
		 */

		/**
		 * a simple structure indicating a chatroom object
		 */
		struct ChatRoom { 
			int number;
			int occupants;
			std::string  name;
			ChatRoom(){
				occupants = 0;
				number = 0;
			}
		};

	private: 
		PurpleRoomlist* roomlist;
	public:
		/**
		 * a lobby object. A lobby can have sub-rooms
		 */
		struct Lobby : public ChatRoom { 
			std::vector<ChatRoom> subrooms;
		};


		/**
		 * initiate a request to get the list of chat rooms. 
		 * Important note.
		 */
		PurpleRoomlist* sendGetChatRoomsRequest();

		/**
		 * callback for chatroomreqesponse. Will also notify libpurple
		 * if necessary. 
		 */
		void parseChatRoomsResponse(MessageBuffer &buffer);
		
	};
	
}

#endif
