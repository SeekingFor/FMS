#ifndef _base64_
#define _base64_

#include <string>
#include <vector>

namespace Base64
{

const bool Encode(const std::vector<unsigned char> &data, std::string &encoded);
const bool Decode(const std::string &encoded, std::vector<unsigned char> &data);
const std::string FreenetBase64ToRealBase64(const std::string &base64val);
const std::string RealBase64ToFreenetBase64(const std::string &base64val);

}	// namespace

#endif	// _base64_
