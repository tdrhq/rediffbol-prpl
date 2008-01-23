#ifndef _GKGETLOGINSERVERSRESPONSE_
#define _GKGETLOGINSERVERSRESPONSE_

#include "response.h" 
#include <glib.h>

struct GkGetLoginServersResponse { 
	struct Response r ; 
	int type ; 
	int subtype ;
	
	/* to be neat and clean, we gotta do this */
	struct MessageBuffer *capabilities ;
};

extern struct GkGetLoginServersResponse *
gkgetloginserversresponse_init(int type, int subtype, GHashTable *map) ;

extern void 
gkgetloginserversresponse_destroy(struct GkGetLoginServersResponse* r) ;

extern void
gkgetloginserversresponse_capability(GString *cap, GString *val) ;

#endif
