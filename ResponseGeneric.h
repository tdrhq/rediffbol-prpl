
#ifndef __RESPONSE_GENERIC_h__
#define __RESPONSE_GENERIC_h__

#include "response.h" 
namespace std { 
	class ResponseCSGeneric : public Response { 
		MessageBuffer data ;
	public: 
		virtual void libpurpleProcess(RediffBolConn* rb)  ; 
		
		virtual bool parsePacket(MessageBuffer &m) ;
		
	}; 
}

#endif
