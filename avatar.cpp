/**
 * @file avatar.cpp 
 * 
 * Copyright (C) 2008-2009 Arnold Noronha <arnstein87 AT gmail DOT com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */



#include "rediffbol.h"
#include <util.h>

using namespace std;
using namespace rbol;

static void load_avatar_callback(PurpleUtilFetchUrlData* url_data, 
				  gpointer user_data, const gchar * url_text, 
				  gsize len ,const gchar* error_message) {
	purple_debug_info ("rbol", "avatar callback\n");
	pair<int, string> *data = NULL;
	data = (typeof(data)) (user_data);
	RediffBolConn* conn = (RediffBolConn*) RObject::getObjectById (data->first);

	if (!conn) { 
		delete data;
		return;
	}

	if (url_text == NULL or conn->isInvalid())  {
		delete data;
		return;
	}

	conn->_loadAvatarCompleted(data->second, string(url_text, 
							       url_text+len));
	delete data;
}


static void load_avatar_key_callback(PurpleUtilFetchUrlData* url_data, 
				  gpointer user_data, const gchar * url_text, 
				  gsize len ,const gchar* error_message) {
	pair<int, string> *data = NULL;
	data = (typeof(data)) (user_data);

	if (! url_text) { 
		purple_debug_info("rbol", "Unable to load url string\n");
		delete data;
		return;
	}

	purple_debug_info("rbol", "Key String: %s\n", url_text);
	string name = data->second;

	string url = "http://imavatars.rediff.com/avatars/getmasque_new.asp?type=image&username=" + name + "&key=";
	
	string r_key  (url_text, url_text+len);
	
	if (r_key.length () < 6) {
		purple_debug_error ("rbol", "avatar string too short\n");
		delete data;
		return;
	}

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
	pair<int, string> *data = new pair<int, string> 
		(this->getId(), name);

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
