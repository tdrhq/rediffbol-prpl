/**
 * @file FontParser.cpp 
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


#include "FontParser.h" 
#include <debug.h>
#include <cstdio>

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


