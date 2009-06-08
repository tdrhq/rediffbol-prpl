/**
 * @file request.h 
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


#ifndef __REQUEST__H__ 
#define __REQUEST__H__ 

namespace rbol { 
	#define GKRequestHeader               "Gatekeeper2"  
	#define GKCmdGetLoginServers          "GetLoginServers"  
	#define CSRequestHeader               "Server1.2"  
	#define CSCmdSignIn                           "SignIn"  
	#define CSCmdGetOfflineMsgs           "GetOfflineMsgs"  
	#define CSCmdSetOnlineStatus          "SetOnlineStatus"  
	#define CSCmdTextMessage              "TextMessage"  
	#define CSPeerRequestHeader           "ClientPeer1.2"  
	#define CSCmdGetChatRooms             "GetChatRoomsRequest"  
	#define CSCmdJoinChatRoom             "JoinChatRoom"  
	#define CSCmdKeepAlive                        "KeepAlive"  
	#define CSCmdDelOfflineMsg            "DelOfflineMsg"  
	#define CSCmdGetContactId             "GetContactId"  
	#define CSCmdLeaveChatRoom            "LeaveChatRoom"  
	#define CSCmdGetAddRequests           "GetAddRequests"  
	#define CSCmdAcceptAdd                        "AcceptAdd"  
	#define CSCmdAcceptAdd2                       "AcceptAdd2"  
	#define CSCmdBlockUser                        "BlockUser"  
	#define CSCmdAllowUser                        "AllowUser"  
	#define CSCmdDelAddRequest            "DelAddRequest"  
	#define CSCmdDelContact               "DelContact2"  
	#define CSCmdAddGroup                 "AddGroup"  
	#define CSCmdRemoveGroup              "RemoveGroup"  
	#define CSCmdMoveContact              "MoveContactBetweenGroups"  
	#define CSCmdAddContact               "AddContact2"  
	#define CSCmdCheckUserType            "CheckUserType"  
	#define CSCmdChatRoomMessage          "ChatRoomMessage"  
	#define CSCmdTypingNotify             "TypingNotify"  

}
#endif

