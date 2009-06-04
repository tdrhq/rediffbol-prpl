
#include "util.h"
#include <string>
#include <glib.h>

using namespace std;


string rbol::intToSWord(int i) { 
	string a;
	a.push_back((i>>8) & 0xff);
	a.push_back(i& 0xff);
	return a;
}


string rbol::intToDWord(int i) { 
	string a;
	a.push_back (i & 0xff);
	a.push_back ((i>>8) & 0xff);
	a.push_back((i>>16) & 0xff);
	a.push_back((i>>24) & 0xff);
	return a;

		     
}

int rbol::bytesToLEInt(char a, char b, char c, char d) { 
	gint32 ret = 0;
	ret |= (a<< 24);
	ret |= (b <<16);
	ret |= (c<< 8);
	ret |= (d);
	return ret;
}
