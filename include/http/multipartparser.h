#ifndef _multipartparser_
#define _multipartparser_

#include <Poco/Net/PartHandler.h>
#include <map>
#include <string>

#include "queryvar.h"

class MultiPartParser:public Poco::Net::PartHandler
{
public:
	void handlePart(const Poco::Net::MessageHeader &header, std::istream &stream);

	std::map<std::string,QueryVar> GetVars()			{ return m_vars; }

private:
	std::map<std::string,QueryVar> m_vars;
};

#endif	// _multipartparser_
