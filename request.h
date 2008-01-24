
#ifndef __REQUEST__H__ 
#define __REQUEST__H__ 
namespace rbol { 
	const char* GKRequestHeader             = "Gatekeeper2" ;
	const char* GKCmdGetLoginServers        = "GetLoginServers" ;
	const char* CSRequestHeader             = "Server1.2" ;
	const char* CSCmdSignIn                         = "SignIn" ;
	const char* CSCmdGetOfflineMsgs         = "GetOfflineMsgs" ;
	const char* CSCmdSetOnlineStatus        = "SetOnlineStatus" ;
	const char* CSCmdTextMessage            = "TextMessage" ;
	const char* CSPeerRequestHeader         = "ClientPeer1.2" ;
	const char* CSCmdGetChatRooms           = "GetChatRoomsRequest" ;
	const char* CSCmdJoinChatRoom           = "JoinChatRoom" ;
	const char* CSCmdKeepAlive                      = "KeepAlive" ;
	const char* CSCmdDelOfflineMsg          = "DelOfflineMsg" ;
	const char* CSCmdGetContactId           = "GetContactId" ;
	const char* CSCmdLeaveChatRoom          = "LeaveChatRoom" ;
	const char* CSCmdGetAddRequests         = "GetAddRequests" ;
	const char* CSCmdAcceptAdd                      = "AcceptAdd" ;
	const char* CSCmdAcceptAdd2                     = "AcceptAdd2" ;
	const char* CSCmdBlockUser                      = "BlockUser" ;
	const char* CSCmdAllowUser                      = "AllowUser" ;
	const char* CSCmdDelAddRequest          = "DelAddRequest" ;
	const char* CSCmdDelContact             = "DelContact2" ;
	const char* CSCmdAddGroup               = "AddGroup" ;
	const char* CSCmdRemoveGroup            = "RemoveGroup" ;
	const char* CSCmdMoveContact            = "MoveContactBetweenGroups" ;
	const char* CSCmdAddContact             = "AddContact2" ;
	const char* CSCmdCheckUserType          = "CheckUserType" ;
	const char* CSCmdChatRoomMessage        = "ChatRoomMessage" ;

}
#endif

