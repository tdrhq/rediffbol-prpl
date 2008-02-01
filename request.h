
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

