#include "../../include/http/multipartparser.h"

#include <Poco/Net/MessageHeader.h>
#include <Poco/StreamCopier.h>

void MultiPartParser::handlePart(const Poco::Net::MessageHeader &header, std::istream &stream)
{
	std::string name="";
	std::string data="";

	if(header.has("Content-Disposition"))
	{
		std::string disp;
		Poco::Net::NameValueCollection nvc;
		Poco::Net::MessageHeader::splitParameters(header["Content-Disposition"],disp,nvc);
		name=nvc.get("name","");

		Poco::StreamCopier::copyToString(stream,data);

		vars[name]=data;
	}
}
