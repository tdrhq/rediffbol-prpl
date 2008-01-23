
#include "response.h" 


struct CSNewMailCountResponse*  newmailcountresponse_init(int n) { 
	struct CSNewMailCountResponse *res = (struct CSNewMailCountResponse*) 
		response_init_protected(RESPONSE_ID_NEWMAILCOUNT, 
					sizeof(struct CSNewMailResponse),
					NULL) ;
	res->count = n ;
	return res ;
}
