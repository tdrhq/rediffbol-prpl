

#include "rediffbol.h"
#include <util.h>

using namespace std;
using namespace rbol;

static void load_avatar_callback(PurpleUtilFetchUrlData* url_data, 
				  gpointer user_data, const gchar * url_text, 
				  gsize len ,const gchar* error_message) {
	purple_debug_info ("rbol", "avatar callback\n");
	pair<RediffBolConn*, string> *data = NULL;
	data = (typeof(data)) (user_data);

	if (url_text == NULL or data ->first->isInvalid())  {
		data->first->delRef();
		delete data;
		return;
	}
	data->first->_loadAvatarCompleted(data->second, string(url_text, 
							       url_text+len));
	data->first->delRef();
	delete data;
}


static void load_avatar_key_callback(PurpleUtilFetchUrlData* url_data, 
				  gpointer user_data, const gchar * url_text, 
				  gsize len ,const gchar* error_message) {
	pair<RediffBolConn*, string> *data = NULL;
	data = (typeof(data)) (user_data);

	if (! url_text) { 
		purple_debug_info("rbol", "Unable to load url string\n");
		data->first->delRef();
		delete data;
		return;
	}

	purple_debug_info("rbol", "Key String: %s\n", url_text);
	string name = data->second;

	string url = "http://imavatars.rediff.com/avatars/getmasque_new.asp?type=image&username=" + name + "&key=";
	
	string r_key  (url_text, url_text+len);
	r_key = r_key.substr(5);

	for(size_t l = 0; l < r_key.size(); l++) 
		if (r_key[l] == '<') { 
			r_key = r_key.substr(0, l);
			break;
		}
	

	url += r_key;


	purple_debug_info("rbol", "Ok, getting avatar for %s\n", name.c_str());
	purple_util_fetch_url_request(strdup(url.c_str()), 
				      true, 
				      NULL, /* useragent */
				      true,  /* http1.1*/
				      NULL, /* request */
				      false, /* don't include headers */
				      load_avatar_callback, 
				      (void*) data);

}
void RediffBolConn::loadAvatar(std::string name) { 
	addRef();
	pair<RediffBolConn*, string> *data = new pair<RediffBolConn*, string> 
		(this, name);

	string url = "http://avatars.rediff.com/avatars/getkey.asp?username="
		+ name + 
		+ "&secret=rediffbol&login=" 
	        + getServerUserId() + "&session_id="
		+ getSessionString();


	purple_util_fetch_url_request(strdup(url.c_str()), 
				      true, 
				      NULL, /* useragent */
				      true,  /* http1.1*/
				      NULL, /* request */
				      false, /* don't include headers */
				      load_avatar_key_callback, 
				      (void*) data);
	


}

void RediffBolConn::_loadAvatarCompleted(std::string name, 
					 std::string avatar_data) { 
	if (isInvalid()) { 
		purple_debug_info("rbol", "Got avatar for invalid object\n");
		return;
	}

	char * data = (char*)malloc(avatar_data.length());
	memcpy(data, (void*) avatar_data.data(), avatar_data.length());
	purple_buddy_icons_set_for_user(account,
					name.c_str(), 
					(void*) data, 
					avatar_data.length(), 
					NULL);
}
