/**
 * @file FontParser.h 
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

#ifndef __fontparse__h__
#define __fontparse__h__ 

#include <string>
#include "config.h"

namespace rbol { 
	class FontParser { 
		std::string name;
		std::string data;
		
	public:
		FontParser(std::string fontname,std::string fontdata);

		int getSize();
		std::string getColor();
		bool isItalic();
		bool isBold();
		bool isStrikedOut();
		bool isUnderlined();
	};
}
#endif
