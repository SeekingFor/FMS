#ifndef _multipartparser_
#define _multipartparser_

#include <Poco/Net/PartHandler.h>
#include <map>
#include <string>

class MultiPartParser:public Poco::Net::PartHandler
{
public:
	void handlePart(const Poco::Net::MessageHeader &header, std::istream &stream);

	std::map<std::string,std::string> GetVars()	{ return vars; }

private:
	std::map<std::string,std::string> vars;
};

#endif	// _multipartparser_
