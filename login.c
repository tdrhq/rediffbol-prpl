
/* do the initial login job */

#define DEFAULTGKIP "203.199.83.62"
#define DEFAULTGKPORT 1836
static void gk_connect_cb(gpointer, gint, const gchar *) ;

void connectToGk(PurpleAccount *account, GString **ar, int *num) { 
	purple_proxy_connect(account, 
			     account,
			     DEFAULTGKIP, 
			     DEFAULTGKPORT,
			     gk_connect_cb, 
			     account);
}

static void gk_connect_cb(gpointer data, gint source, const gchar *err) {
	PurpleAccount* account = data ;
	PurpleConnection *gc = account->gc ; 
	struct RediffBolConn * rb = account->proto_data ; 
	if ( source < 0 ) { 
		purple_debug(PURPLE_DEBUG_ERROR, "rbol",
			     "Unable to connect\n") ;
		/* todo */
	}

	rb->fd = source ;
	
}
void startLoginProcess(PurpleAccount * account) { 
	

}
