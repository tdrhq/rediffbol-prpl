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


struct Response ;
typedef void(*RESPONSE_DESTRUCTOR)(struct Response*) ;

struct Response { 
	int responseId; 
	RESPONSE_DESTRUCTOR *destructor ; 
	void* data[1] ; 
} ;

extern struct Response* 
response_init_protected(int respId, int datalen, RESPONSE_DESTRUCTOR *rd) ;

extern void response_free(struct Response *r) ;


#endif
