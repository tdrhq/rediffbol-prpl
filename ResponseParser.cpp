
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

Response* 
parseResponse(MessageBuffer &buffer, int requesttype ) { 
	try { 
		switch(requesttype)  {
		case GK_REQUEST: 
			GkGetLoginServersResponse *resp  = new 
				GkGetLoginServersResponse ; 
			resp->parseResponse(buffer) ;
			return resp ;
		case CS_REQUEST:
			return parseCSResponse(buffer) ;
			break ; 
		}
	} catch( MessageBufferOverflowException e ) { 
		buffer.reset() ;
		return NULL ;
	}

	return NULL ;

}


parseGeneric(struct MessageBuffer *buffer) { 
	int size = messagebuffer_readInt(buffer) ;
	GString *tmp =
		messagebuffer_readStringn(buffer, size) ;
	g_string_free(tmp, TRUE) ;
		
	if ( buffer->err) 
		return NULL ;
	return response_init_protected(0, 0, NULL) ;
}

static struct Response* 
parseServerMessage(struct MessageBuffer *buffer) { 
	int code = messagebuffer_readInt(buffer);
	return parseGeneric(buffer) ; /* todo */ 
}
struct Response*
parseCSResponse(struct MessageBuffer *buffer) { 
	struct Response *resp = NULL ;
	GString *header = NULL ;
	GString *cmd = NULL ;

	int type = messagebuffer_readInt(buffer) ;
	if ( buffer->err) return NULL ;

	switch(type) { 
	case 1: 
		if ( buffer->str->len == 12 ) {
			/* TODO: this is not a good way of checking for
			   the length of the packet size */
			gchar* hex_dump = purple_str_binary_to_ascii
				(buffer->str->str, 
				 buffer->str->len) ;
			purple_debug(PURPLE_DEBUG_INFO, "rbol", 
				     "Got keepalive response: \n%s\n",
				     hex_dump) ;
			g_free(hex_dump) ;
			return response_init_protected(RESPONSE_ID_KEEPALIVE, 
						       0, NULL) ;
		}
		int subtype = messagebuffer_readInt(buffer) ;
		if( buffer->err ) return NULL ;

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
			resp = parseGeneric(buffer) ; ;
		}
		return resp ; 
	case 3: 
		header = messagebuffer_readString(buffer) ;
		cmd = messagebuffer_readString(buffer) ;

		if ( buffer-> err ) return NULL ;
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
			resp = parseGeneric(buffer) ;
		}
		g_string_free(header, TRUE) ;
		g_string_free(cmd, TRUE);
		return resp ; 
	case 2: 
		/* server message */
		resp = parseServerMessage(buffer) ; 
		return resp ; 
	}
				
		
	
}
