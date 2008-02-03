#include "../../../include/http/pages/showcaptchapage.h"
#include "../../../include/base64.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ShowCaptchaPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="HTTP/1.1 200 OK\r\n";
	if(queryvars.find("UUID")!=queryvars.end())
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT MimeType,PuzzleData FROM tblIntroductionPuzzleRequests WHERE UUID=?;");
		st.Bind(0,(*queryvars.find("UUID")).second);
		st.Step();

		if(st.RowReturned())
		{
			std::string mime;
			std::string b64data;
			std::vector<unsigned char> data;
			std::string lenstr;

			st.ResultText(0,mime);
			st.ResultText(1,b64data);
			Base64::Decode(b64data,data);
			StringFunctions::Convert(data.size(),lenstr);

			content+="Content-Type: "+mime+"\r\n";
			content+="Content-Length: "+lenstr+"\r\n\r\n";
			content+=std::string(data.begin(),data.end());
		}
	}
	return content;
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
