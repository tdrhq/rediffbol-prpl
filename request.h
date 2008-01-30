
#ifndef __REQUEST__H__ 
#define __REQUEST__H__ 
namespace rbol { 
	static const char* GKRequestHeader             = "Gatekeeper2" ;
	static const char* GKCmdGetLoginServers        = "GetLoginServers" ;
	static const char* CSRequestHeader             = "Server1.2" ;
	static const char* CSCmdSignIn                         = "SignIn" ;
	static const char* CSCmdGetOfflineMsgs         = "GetOfflineMsgs" ;
	static const char* CSCmdSetOnlineStatus        = "SetOnlineStatus" ;
	static const char* CSCmdTextMessage            = "TextMessage" ;
	static const char* CSPeerRequestHeader         = "ClientPeer1.2" ;
	static const char* CSCmdGetChatRooms           = "GetChatRoomsRequest" ;
	static const char* CSCmdJoinChatRoom           = "JoinChatRoom" ;
	static const char* CSCmdKeepAlive                      = "KeepAlive" ;
	static const char* CSCmdDelOfflineMsg          = "DelOfflineMsg" ;
	static const char* CSCmdGetContactId           = "GetContactId" ;
	static const char* CSCmdLeaveChatRoom          = "LeaveChatRoom" ;
	static const char* CSCmdGetAddRequests         = "GetAddRequests" ;
	static const char* CSCmdAcceptAdd                      = "AcceptAdd" ;
	static const char* CSCmdAcceptAdd2                     = "AcceptAdd2" ;
	static const char* CSCmdBlockUser                      = "BlockUser" ;
	static const char* CSCmdAllowUser                      = "AllowUser" ;
	static const char* CSCmdDelAddRequest          = "DelAddRequest" ;
	static const char* CSCmdDelContact             = "DelContact2" ;
	static const char* CSCmdAddGroup               = "AddGroup" ;
	static const char* CSCmdRemoveGroup            = "RemoveGroup" ;
	static const char* CSCmdMoveContact            = "MoveContactBetweenGroups" ;
	static const char* CSCmdAddContact             = "AddContact2" ;
	static const char* CSCmdCheckUserType          = "CheckUserType" ;
	static const char* CSCmdChatRoomMessage        = "ChatRoomMessage" ;
	static const char* CSCmdTypingNotify           = "TypingNotify" ;

}
#endif

