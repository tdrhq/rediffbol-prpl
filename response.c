
#include "response.h"

struct Response* 
response_init_protected(int respId, int datalen, RESPONSE_DESTRUCTOR *rd) {
	struct Response *ret = g_new(struct Response, 1) ;
	ret->responseId = respId ;
	if ( datalen ) 
		ret->data = g_malloc(datalen) ;
	else 
		ret->data = NULL ;
	ret->destructor = rd ;
}
void response_free(struct Response *r) { 
	if ( r->destructor ) 
		(*r->destructor)(r) ;
	if ( r->data) g_free(r->data) ;
	g_free(r) ;
}
