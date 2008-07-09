#include "../include/global.h"
#include "../include/stringfunctions.h"

std::string CreateShortIdentityName(const std::string &name, const std::string &publickey)
{
	std::string result="";
	std::vector<std::string> keyparts;

	StringFunctions::SplitMultiple(publickey,"@,",keyparts);

	result+=name;
	if(keyparts.size()>1 && keyparts[1].size()>8)
	{
		result+="@"+keyparts[1].substr(0,4)+"...";
	}

	return result;
}
