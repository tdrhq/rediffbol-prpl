/**
 * @file util.h 
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

#ifndef __UTIL__H__
#define __UTIL__H__ 

#include <stdlib.h>
#include <string> 

namespace rbol { 
	std::string intToDWord(int a);
	std::string intToSWord(int a);
	std::string intToDWordLE(int a);
	std::string bytes2Int(std::string a);
	
	std::string bytes2Int(char byte0, char byte1, char byte2, 
			      char byte3);

	int bytesToLEInt (char byte0, char byte1, char byte2, 
				  char byte3);

	int bytes2Short(std::string buf);
}

#endif
