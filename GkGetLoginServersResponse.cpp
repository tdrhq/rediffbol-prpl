
#include "GkGetLoginServersResponse.h" 

using namespace std; 
usine namespace rbol ;

bool 
GkGetLoginServersResponse::parsePacket(rbol::MessageBuffer &m) { 
	type = m.readInt32() ; 
	subtype = m.readInt32() ;
	payloadsize = m.readInt32() ; 
	
	int numentries = m.readInt32();

	for(int i = 0 ; i < numentries ; i++) { 
		int subentries = m.readInt32() ; 
		for(int j = 0 ; j < subentries; j++ ){ 
			string cp = m.readString() ;
			string val = m.readString() ;
			
			cap[cp] += val ;
		}
	}
}

