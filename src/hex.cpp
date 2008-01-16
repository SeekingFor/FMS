#include "../include/hex.h"

#ifdef XMEM
	#include <xmem.h>
#endif

namespace Hex
{
	
static const std::string hexchars="0123456789ABCDEF";

const bool Encode(const std::vector<unsigned char> &data, std::string &encoded)
{
	for(std::vector<unsigned char>::const_iterator i=data.begin(); i!=data.end(); i++)
	{
		encoded.push_back(hexchars[(((*i)>>4) & 0x0F)]);
		encoded.push_back(hexchars[((*i) & 0x0F)]);
	}
	return true;
}

const bool Decode(const std::string &encoded, std::vector<unsigned char> &data)
{

	std::string::size_type pos=0;
	unsigned char byte;
	int bytepart=0;
	
	pos=encoded.find_first_of(hexchars);
	
	while(pos!=std::string::npos)
	{
		if(bytepart==0)
		{
			byte=(hexchars.find(encoded[pos]) << 4) & 0xF0;
			bytepart=1;
		}
		else
		{
			byte|=hexchars.find(encoded[pos]) & 0x0F;
			data.push_back(byte);
			bytepart=0;
		}
		pos=encoded.find_first_of(hexchars,pos+1);
	}
	return true;
}

}	// namespace
