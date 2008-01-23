
#include "response.h"

struct Response* 
response_init_protected(int respId, int datalen, RESPONSE_DESTRUCTOR *rd) {
	if ( datalen == 0 ) datalen = sizeof(struct Response) ;
	struct Response *ret = g_malloc(datalen) ;
	ret->responseId = respId ;
	ret->destructor = rd ;
}
void response_free(struct Response *r) { 
	if ( r->destructor ) 
		(*r->destructor)(r) ;
	if ( r->data) g_free(r->data) ;
	g_free(r) ;
}
