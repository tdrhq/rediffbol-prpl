
#include "FontParser.h" 
#include <debug.h>

using namespace std;
using namespace rbol;

FontParser::FontParser(string fontname, string fontdata) { 
	name = fontname;
	data = fontdata;
}

int FontParser::getSize() { 
	int v =  (int)(0xff & data[1]);
	return v;
}

string FontParser::getColor()  { 
	char color[100];
	
	int nc = data[10];
	nc <<= 8;

	nc |= data[11];
	nc <<= 8;

	nc |= data[12];
	
	sprintf(color, "#%06x", nc);
	
	purple_debug_info("rbol", "Got color as %s\n", color);

	return string(color);
}

bool FontParser::isItalic()  { 
	return data[8] & 0x01;
}

bool FontParser::isBold() { 
	int bold = data[7] & 0xff;
	return (bold == 75);
}

bool FontParser::isStrikedOut() { 
	return data[8] & 0x04;
}

bool FontParser::isUnderlined() {
	return data[8] & 0x02;
}


