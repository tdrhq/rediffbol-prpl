#ifndef _GKGETLOGINSERVERSRESPONSE_
#define _GKGETLOGINSERVERSRESPONSE_

#include "response.h" 
#include "messagebuffer.h" 

#include <glib.h>
#include <list>
#include <map>
namespace rbol { 
	class GkGetLoginServersResponse  : public Response{ 
		int type ; 
		int subtype ;
		int payloadsize ; 
		std::map<std::string,std::string> cap ; 

		
	public:
		GkGetLoginServersResponse() { 
		}

		virtual bool parsePacket(rbol::MessageBuffer &m) ;
		std::list<std::string> getDirectConnectionIP() ; 

	};
}

#endif
