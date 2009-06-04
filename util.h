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

	int bytesToLEInt ( char byte0, char byte1, char byte2, 
				  char byte3);

	int bytes2Short(std::string buf);
}

#endif
