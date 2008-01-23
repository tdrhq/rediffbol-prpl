
#include "GkGetLoginServersResponse.h" 

struct GkGetLoginServersResponse *
gkgetloginserversresponse_init(int type, int subtype, GHashTable *map) { 
	struct GkGetLoginServersResponse *ret = 
		(struct GkGetLoginServersResponse*) 
		response_init_protected(RESPONSE_ID_GKGETLOGINSERVERS,
					sizeof(struct GkGetLoginServersResponse),
					(RESPONSE_DESTRUCTOR)(gkgetloginserverresponse_destroy)) ;
				     
}

void gkgetloginserversresponse_destroy(struct GkGetLoginServersResponse* r) {
	if (r->map) g_hash_table_destroy(r->map) ;
}

extern void
gkgetloginserversresponse_capability(GString *cap, GString *val) { 
	
}
