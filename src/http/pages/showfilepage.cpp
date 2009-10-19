#include "../../../include/http/pages/showfilepage.h"

#include <cstdio>

#ifdef XMEM
	#include <xmem.h>
#endif

std::map<std::string,std::string> ShowFilePage::m_filewhitelist;

ShowFilePage::ShowFilePage(SQLite3DB::DB *db):IPageHandler(db)
{
	m_filewhitelist["images/new_posts.png"]="image/png";
	m_filewhitelist["images/no_new_posts.png"]="image/png";
	m_filewhitelist["images/mail_generic.png"]="image/png";
	m_filewhitelist["images/mail_new3.png"]="image/png";
	m_filewhitelist["images/attach.png"]="image/png";
	m_filewhitelist["images/mail_reply.png"]="image/png";
	m_filewhitelist["images/mail_get.png"]="image/png";
	m_filewhitelist["images/circleplus.png"]="image/png";
	m_filewhitelist["images/circleminus.png"]="image/png";
	m_filewhitelist["styles/basestyle.css"]="text/css";
	m_filewhitelist["styles/forumstyle.css"]="text/css";
}

void ShowFilePage::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("ShowFilePage::handleRequest from "+request.clientAddress().toString());

	std::map<std::string,std::string> queryvars;
	CreateQueryVarMap(request,queryvars);

	std::string content="";
	if(queryvars.find("file")!=queryvars.end() && m_filewhitelist.find((*queryvars.find("file")).second)!=m_filewhitelist.end())
	{
		try
		{
			response.sendFile((*queryvars.find("file")).second,m_filewhitelist[(*queryvars.find("file")).second]);
		}
		catch(...)
		{

		}
	}
	else if(request.getURI().size()>0 && request.getURI()[0]=='/' && m_filewhitelist.find(request.getURI().substr(1))!=m_filewhitelist.end())
	{
		try
		{
			response.sendFile(request.getURI().substr(1),m_filewhitelist[request.getURI().substr(1)]);
		}
		catch(...)
		{

		}		
	}

}

const bool ShowFilePage::WillHandleURI(const std::string &uri)
{
	if(uri.find("showfile.htm")!=std::string::npos || (uri.size()>1 && uri[0]=='/' && m_filewhitelist.find(uri.substr(1))!=m_filewhitelist.end()))
	{
		return true;
	}
	else
	{
		return false;
	}
}
