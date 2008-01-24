#ifndef __REDIFFBOL_H__
#define __REDIFFBOL_H__ 

#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <glib.h>

#include "account.h"
#include "accountopt.h"
#include "blist.h"
#include "debug.h"
#include "notify.h"
#include "privacy.h"
#include "prpl.h"
#include "roomlist.h"
#include "status.h"
#include "util.h"
#include "version.h"
#include <curl/curl.h>
#include <assert.h>
#include "xmlnode.h"
#include <ctype.h>


typedef void (*GcFunc)(PurpleConnection *from,
                       PurpleConnection *to,
                       gpointer userdata);
namespace rbol { 
	
	class RediffBolConn { 
	public: 
		int fd ; 
		PurpleAccount *account ;

		RediffBolConn(PurpleAccount*) ;
		virtual void responseCallback(Response* resp) ;
		PurpleAsyncConn* getAsyncConnection() ; 
	};
	
}

#endif
