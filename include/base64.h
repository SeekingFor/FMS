#ifndef _base64_
#define _base64_

#include <string>
#include <vector>

namespace Base64
{

const bool Encode(const std::vector<unsigned char> &data, std::string &encoded);
const bool Decode(const std::string &encoded, std::vector<unsigned char> &data);

}	// namespace

#endif	_base64_
