/**
 * @file response.h 
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
	class RediffBolConn;
	
	class Response { 
		int responseId;
	public:
		virtual int getResponseId()  {
			return responseId;
		}
		
		/* this is the actual processing call. This _will_ interact
		 *  with libpurple and make changes. */
		virtual void libpurpleProcess(RediffBolConn* rb) = 0;
		
		virtual bool parsePacket(MessageBuffer &m) = 0;
		virtual ~Response() {};
	};
}

extern void response_free(struct Response *r);


#endif
