
#define GK_REQUEST 1 
#define CS_REQUEST 2 

#include "messagebuffer.h"
#include "response.h"
#include "GkGetLoginServersResponse.h"

#include <glib.h>
#include <libpurple/debug.h>
#include <libpurple/util.h>

/**
 * Return's NULL if we didn't get a valid Response 
 */

static Response* parseGKResponse(MessageBuffer buffer) ;
static Response* parseCSResponse(MessageBuffer buffer) ;
using namespace rbol ; 
using namespace std; 


#include "rediffbol.h" 

bool 
RediffBolConn::parseResponse(MessageBuffer &buffer, int requesttype ) { 
	Response *_resp = NULL; 
	try { 
		switch(requesttype)  {
		case GK_REQUEST: 
			GkGetLoginServersResponse *resp  = new 
				GkGetLoginServersResponse ; 
			_resp = resp ;
			resp->parseResponse(buffer) ;
			return resp ;
		case CS_REQUEST:
			return parseCSResponse(buffer) ;
			break ; 
		}
	} catch( MessageBufferOverflowException e ) { 
		if ( _resp ) delete _resp ; 
		buffer.reset() ;
		return NULL ;
	}

	return NULL ;

}

static struct Response* 
parseServerMessage(struct MessageBuffer *buffer) { 
	int code = messagebuffer_readInt(buffer);
	return parseGeneric(buffer) ; /* todo */ 
}
struct Response*
parseCSResponse(MessageBuffer &buffer) { 
	Response *resp = NULL ;
	string header = NULL ;
	string cmd = NULL ;

	int type = buffer.readInt() ;

	switch(type) { 
	case 1: 
		if ( buffer.getLength() == 12 ) {
			/* TODO: this is not a good way of checking for
			   the length of the packet size */
			gchar* hex_dump = purple_str_binary_to_ascii
				(buffer->str->str, 
				 buffer->str->len) ;
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "Got keepalive response: \n%s\n",
				     hex_dump) ;
			g_free(hex_dump) ;
			buffer.readInt32() ;
			buffer.readInt32(); 
			return NULL ;
		}

		int subtype = buffer.readInt() ;

		if ( subtype == 0 ) { 
			/* login response */
		} else if (subtype == 1 ) { 
			/* offline messages */
		} else if ( subtype == 7 ) { 
			/* chat room response */
		} else if ( subtype == 62 ) { 
			/* join chat room response */
		}

		if ( !resp ) { 
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "Got subtype as %d\n", subtype) ;
			resp = new ResponseCSGeneric ; 
		}

		try { 
			resp->parseResponse(buffer);
		} catch ( MessageBufferOverflowException e ) { 
			delete resp ; 
			return NULL ;
		}
		
		return resp ; 
	case 3: 
		header = buffer.readString(buffer) ;
		cmd = buffer.readString(buffer) ;

		purple_debug(PURPLE_DEBUG_INFO, "rbol", 
			     "cmd is %s\n", cmd->str) ;
		resp = NULL ; 

		if ( g_strcmp(cmd->str, "Contacts") == 0 ) { 
			/* resp = parseContacts(buffer); */
		} else if ( g_strcmp(cmd->str, "NewMailCount") == 0) { 
			/* resp = parseNewMailCount */
		}
		
		if ( !resp) { 
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "Unhandled\n"); 
			resp = new ResponseCSGeneric ;
		}
		
		try { 
			resp->parseResponse(buffer) ;
		} catch (MessageBufferOverflowException e ) {
			delete resp ; 
			return NULL ; 
		}
		return resp ; 
	case 2: 
		/* server message */
		int code = buffer.readInt32() ; 
		int size = buffer.readInt32() ; 
		
		string message = buffer.readString() ; 
		string reason = buffer.readString() ; 
		int code2 = buffer.readInt() ; 

		purple_debug(PURPLE_DEBUG_INFO, "SERVER MESSAGE: code=%d code2="
			     "%d MESSAGE=%s | REASON=%s\n",
			     code, code2, message.c_str() , reason.c_str() );

		return NULL ;
	}
				
		
	
}
