#ifndef __ENCODE__H__
#define __ENCODE__H__ 
#include <string>
#include "config.h"
namespace rbol { 
	std::string encode(const std::string a, const std::string from, 
			   const std::string to) ;
	std::string encode_from_iso(const std::string a, 
				    const std::string to) ; 
}

#endif 
