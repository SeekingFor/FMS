#ifndef _hex_funcs_
#define _hex_funcs_

#include <string>
#include <vector>

namespace Hex
{
	
const bool Encode(const std::vector<unsigned char> &data, std::string &encoded);
const bool Decode(const std::string &encoded, std::vector<unsigned char> &data);
		
};

#endif	// _hex_funcs_
