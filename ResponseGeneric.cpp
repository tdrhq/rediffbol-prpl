
#include "ResponseGeneric.h"

virtual void 
ResponseCSGeneric::libpurpleProcess(RediffBolConn* rb)  {
	
}

virtual bool 
ResponseCSGeneric::parsePacket(MessageBuffer &m) { 
	int size = m.readInt() ;
	string tmp = m.readStringn(size) ;

	data = MessageBuffer(tmp) ; 
}
