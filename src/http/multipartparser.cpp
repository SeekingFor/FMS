#include "../../include/http/multipartparser.h"

#include <Poco/Net/MessageHeader.h>
#include <Poco/StreamCopier.h>

void MultiPartParser::handlePart(const Poco::Net::MessageHeader &header, std::istream &stream)
{
	QueryVar qv;
	std::string data;

	if(header.has("Content-Disposition"))
	{
		std::string disp;
		Poco::Net::NameValueCollection nvc;
		Poco::Net::MessageHeader::splitParameters(header["Content-Disposition"],disp,nvc);
		qv.SetName(nvc.get("name",""));
		qv.SetFileName(nvc.get("filename",""));
		if(header.has("Content-Type"))
		{
			qv.SetContentType(header["Content-Type"]);
		}
		else
		{
			qv.SetContentType("");
		}

		Poco::StreamCopier::copyToString(stream,data);
		qv.SetData(data);

		m_vars[qv.GetName()]=qv;
	}
}
