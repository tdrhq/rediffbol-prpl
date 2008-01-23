
#include <glib.h>

struct Response ;
typedef void(*RESPONSE_DESTRUCTOR)(struct Response*) ;

struct Response { 
	int responseId; 
	RESPONSE_DESTRUCTOR *destructor ; 
	void* data ; 
} ;


extern struct Response* 
response_init_protected(int respId, int datalen, RESPONSE_DESTRUCTOR *rd) ;

extern void response_free(struct Response *r) ;
