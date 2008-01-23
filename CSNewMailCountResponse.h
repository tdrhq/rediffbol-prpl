
#include "response.h" 

struct CSNewMailCountResponse { 
	struct Response h ; 
	int count ; 
} ;

extern  struct CSNewMailCountResponse*  
newmailcountresponse_init(int n) ;
