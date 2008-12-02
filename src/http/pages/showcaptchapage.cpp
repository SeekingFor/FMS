#include "../../../include/http/pages/showcaptchapage.h"
#include "../../../include/base64.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

void ShowCaptchaPage::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("ShowCaptchaPage::handleRequest from "+request.clientAddress().toString());

	std::map<std::string,std::string> queryvars;
	CreateQueryVarMap(request,queryvars);

	if(request.getVersion()==Poco::Net::HTTPRequest::HTTP_1_1)
	{
		response.setChunkedTransferEncoding(true);
	}

	std::string content="";
	if(queryvars.find("UUID")!=queryvars.end())
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT MimeType,PuzzleData FROM tblIntroductionPuzzleRequests WHERE Type='captcha' AND UUID=?;");
		st.Bind(0,(*queryvars.find("UUID")).second);
		st.Step();

		if(st.RowReturned())
		{
			std::string mime;
			std::string b64data;
			std::vector<unsigned char> data;

			st.ResultText(0,mime);
			st.ResultText(1,b64data);
			Base64::Decode(b64data,data);

			// mime type should be short and have a / in it - otherwise skip
			if(mime.size()<50 && mime.find("/")!=std::string::npos)
			{
				response.setContentType(mime);
				response.setContentLength(data.size());
				content+=std::string(data.begin(),data.end());
			}
		}
	}

	std::ostream &ostr = response.send();
	ostr << content;
}

const bool ShowCaptchaPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("showcaptcha.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
