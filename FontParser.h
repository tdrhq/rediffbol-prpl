#ifndef __fontparse__h__
#define __fontparse__h__ 

#include <string>
#include "config.h"

namespace rbol { 
	class FontParser { 
		std::string name;
		std::string data;
		
	public:
		FontParser(std::string fontname,std::string fontdata );

		int getSize();
		std::string getColor();
		bool isItalic();
		bool isBold();
		bool isStrikedOut();
		bool isUnderlined();
	};
}
#endif
