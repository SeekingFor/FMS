#include "../../../include/http/pages/homepage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/global.h"
#include "../../../include/fmsapp.h"
#include "../../../include/option.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string HomePage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{

	std::string messagecountstr="";
	std::string filecountstr="";
	std::string fcphost="127.0.0.1";
	std::string fproxyport="8888";

	Option::Instance()->Get("FCPHost",fcphost);
	Option::Instance()->Get("FProxyPort",fproxyport);

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="shutdown" && ValidateFormPassword(queryvars))
	{
		m_log->trace("HomePage::GeneratePage requested shutdown");
		((FMSApp *)&FMSApp::instance())->Terminate();
	}

	std::string content="<h2>Home</h2>";
	content+="<p class=\"paragraph\">";
	content+="<b>FMS version ";
	content+=FMS_VERSION;
	content+="</b><br>";

	bool showgenericupdate=true;
	SQLite3DB::Statement st=m_db->Prepare("SELECT Major, Minor, Release, PageKey FROM tblFMSVersion ORDER BY Major DESC, Minor DESC, Release DESC LIMIT 1;");
	st.Step();
	if(st.RowReturned())
	{
		int major=0;
		int minor=0;
		int release=0;
		int currentmajor=0;
		int currentminor=0;
		int currentrelease=0;
		std::string freesite="";
		std::string majorstr="";
		std::string minorstr="";
		std::string releasestr="";

		StringFunctions::Convert(VERSION_MAJOR,currentmajor);
		StringFunctions::Convert(VERSION_MINOR,currentminor);
		StringFunctions::Convert(VERSION_RELEASE,currentrelease);

		st.ResultInt(0,major);
		st.ResultInt(1,minor);
		st.ResultInt(2,release);
		st.ResultText(3,freesite);

		StringFunctions::Convert(major,majorstr);
		StringFunctions::Convert(minor,minorstr);
		StringFunctions::Convert(release,releasestr);

		if(currentmajor<major || (currentmajor==major && currentminor<minor) || (currentmajor==major && currentminor==minor && currentrelease<release))
		{
			content+="<b>You are running an old version of FMS.  Please update here: <a href=\"http://"+fcphost+":"+fproxyport+"/"+freesite+"\">FMS "+majorstr+"."+minorstr+"."+releasestr+"</a></b><br>";
			content+="You can see the release info <a href=\"versioninfo.htm?Major="+majorstr+"&Minor="+minorstr+"&Release="+releasestr+"\">here</a><br>";
			showgenericupdate=false;
		}

	}

	if(showgenericupdate)
	{
		content+="Check for new versions at the <a href=\"http://"+fcphost+":"+fproxyport+"/USK@0npnMrqZNKRCRoGojZV93UNHCMN-6UU3rRSAmP6jNLE,~BG-edFtdCC1cSH4O3BWdeIYa8Sw5DfyrSV-TKdO5ec,AQACAAE/fms/76/\">FMS Freesite</a><br>";
	}

	content+="Use these pages to administer your FMS installation.";
	content+="</p>";

	st=m_db->Prepare("SELECT COUNT(*) FROM tblMessageInserts WHERE Inserted='false';");
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,messagecountstr);
	}
	content+="Messages waiting to be inserted:"+messagecountstr+"<br>";
	st=m_db->Prepare("SELECT COUNT(*) FROM tblFileInserts WHERE Key IS NULL;");
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,filecountstr);
	}
	content+="Files waiting to be inserted:"+filecountstr+"<br>";

	content+="<p class=\"paragraph\">";
	content+="<form name=\"frmshutdown\" method=\"POST\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"shutdown\">";
	content+="<input type=\"submit\" value=\"Shutdown FMS\">";
	content+="</form>";
	content+="</p>";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool HomePage::WillHandleURI(const std::string &uri)
{
	return true;
}
