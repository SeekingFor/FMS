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

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="shutdown")
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
		std::string currentmajor=VERSION_MAJOR;
		std::string currentminor=VERSION_MINOR;
		std::string currentrelease=VERSION_RELEASE;
		std::string major="";
		std::string minor="";
		std::string release="";
		std::string freesite="";
		st.ResultText(0,major);
		st.ResultText(1,minor);
		st.ResultText(2,release);
		st.ResultText(3,freesite);

		if(currentmajor<major || (currentmajor==major && currentminor<minor) || (currentmajor==major && currentminor==minor && currentrelease<release))
		{
			content+="<b>You are running an old version of FMS.  Please update here: <a href=\"http://"+fcphost+":"+fproxyport+"/"+freesite+"\">FMS "+major+"."+minor+"."+release+"</a></b><br>";
			content+="You can see the release info <a href=\"versioninfo.htm?Major="+major+"&Minor="+minor+"&Release="+release+"\">here</a><br>";
			showgenericupdate=false;
		}

	}

	if(showgenericupdate)
	{
		content+="Check for new versions at the <a href=\"http://"+fcphost+":"+fproxyport+"/USK@0npnMrqZNKRCRoGojZV93UNHCMN-6UU3rRSAmP6jNLE,~BG-edFtdCC1cSH4O3BWdeIYa8Sw5DfyrSV-TKdO5ec,AQACAAE/fms/66/\">FMS Freesite</a><br>";
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
