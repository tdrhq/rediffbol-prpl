#ifndef _RB_RESPONSE_H_
#define _RB_RESPONSE_H_
#include <glib.h>


#define RESPONSE_ID_GKGETLOGINSERVERS    1 
#define RESPONSE_ID_LOGIN                2 
#define RESPONSE_ID_CONTACTS             3 
#define RESPONSE_ID_NEWMAILCOUNT         4 
#define RESPONSE_ID_CONTACT_STATUS_CHG   5 
#define RESPONSE_ID_TEXTMESSAGE          6 
#define RESPONSE_ID_CHATROOMLIST         7 
#define RESPONSE_ID_NICKNAMEUPDATE       8 
#define RESPONSE_ID_KEEPALIVE            9 
#define RESPONSE_ID_OFFLINEMSGS          10 
#define RESPONSE_ID_JOINCHATROOM         11 
#define RESPONSE_ID_CHATROOMMSG          12 
#define RESPONSE_ID_CHATROOMEVENT        13 
#define RESPONSE_ID_ADDREQ               14 
#define RESPONSE_ID_INVALIDUSER          15 
#define RESPONSE_ID_CONTACT_DISP_CHG     16 
#define RESPONSE_ID_CONTACT_ADD_REQ      17 
#define RESPONSE_ID_TYPING_NOTIFY        18 
#define RESPONSE_ID_ACCEPTADD            19 

#include "conn.h"
#include "messagebuffer.h"
#include "rediffbol.h"

namespace rbol { 
	class RediffBolConn ;
	
	class Response { 
		int responseId; 
	public:
		virtual int getResponseId()  {
			return responseId ; 
		}
		
		/* this is the actual processing call. This _will_ interact
		 *  with libpurple and make changes. */
		virtual void libpurpleProcess(RediffBolConn* rb) = 0 ; 
		
		virtual bool parsePacket(MessageBuffer &m) = 0;
		virtual ~Response() {} ;
	} ;
}

extern void response_free(struct Response *r) ;


#endif
