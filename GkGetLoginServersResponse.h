#ifndef _GKGETLOGINSERVERSRESPONSE_
#define _GKGETLOGINSERVERSRESPONSE_

#include "response.h" 
#include "messagebuffer.h" 

#include <glib.h>

namespace rbol { 
	class GkGetLoginServersResponse  : public Response{ 
		int type ; 
		int subtype ;
		int payloadsize ; 
		map<string,string> cap ; 

		
	public:
		GkGetLoginServersResponse() { 
			buf = b ;
		}

		virtual bool parsePacket(rbol::MessageBuffer &m) ;


	};
}

#endif
