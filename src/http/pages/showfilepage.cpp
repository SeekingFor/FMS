#include "../../../include/http/pages/showfilepage.h"

#include <cstdio>
#include <sstream>

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
	m_filewhitelist["images/latest_reply.png"]="image/png";
	m_filewhitelist["images/link.png"]="image/png";
	m_filewhitelist["images/search.png"]="image/png";
	m_filewhitelist["styles/basestyle.css"]="text/css";
	m_filewhitelist["styles/forumstyle.css"]="text/css";
	
	// add smiley images
	for(int i=1; i<=73; i++)
	{
		std::ostringstream numstr;
		numstr << i;
		m_filewhitelist["images/smilies/"+numstr.str()+".gif"]="image/gif";
	}
}

void ShowFilePage::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("ShowFilePage::handleRequest from "+request.clientAddress().toString());
	m_log->trace("ShowFilePage::handleRequest ContentType="+request.getContentType());
	m_log->trace("ShowFilePage::handleRequest TransferEncoding="+request.getTransferEncoding());
	m_log->trace("ShowFilePage::handleRequest URI="+request.getURI());

	std::map<std::string,QueryVar> queryvars;

	CreateQueryVarMap(request,queryvars);

	std::string content="";
	if(queryvars.find("file")!=queryvars.end() && m_filewhitelist.find((*queryvars.find("file")).second.GetData())!=m_filewhitelist.end())
	{
		try
		{
			response.sendFile(global::basepath+(*queryvars.find("file")).second.GetData(),m_filewhitelist[(*queryvars.find("file")).second.GetData()]);
		}
		catch(Poco::FileNotFoundException &fnf)
		{
			m_log->error("ShowFilePage::handleRequest caught FileNotFound exception - "+fnf.message());
		}
		catch(Poco::OpenFileException &of)
		{
			m_log->error("ShowFilePage::handleRequest caught OpenFile exception - "+of.message());
		}
		catch(...)
		{
			m_log->error("ShowFilePage::handleRequest caught other exception");
		}
	}
	else if(request.getURI().size()>0 && request.getURI()[0]=='/' && m_filewhitelist.find(request.getURI().substr(1))!=m_filewhitelist.end())
	{
		try
		{
			response.sendFile(global::basepath+request.getURI().substr(1),m_filewhitelist[request.getURI().substr(1)]);
		}
		catch(Poco::FileNotFoundException &fnf)
		{
			m_log->error("ShowFilePage::handleRequest caught FileNotFound exception - "+fnf.message());
		}
		catch(Poco::OpenFileException &of)
		{
			m_log->error("ShowFilePage::handleRequest caught OpenFile exception - "+of.message());
		}
		catch(...)
		{
			m_log->error("ShowFilePage::handleRequest caught other exception");
		}		
	}
	else if(request.getURI().size()>0 && m_filewhitelist.find(request.getURI())!=m_filewhitelist.end())
	{
		try
		{
			response.sendFile(global::basepath+request.getURI(),m_filewhitelist[request.getURI()]);
		}
		catch(Poco::FileNotFoundException &fnf)
		{
			m_log->error("ShowFilePage::handleRequest caught FileNotFound exception - "+fnf.message());
		}
		catch(Poco::OpenFileException &of)
		{
			m_log->error("ShowFilePage::handleRequest caught OpenFile exception - "+of.message());
		}
		catch(...)
		{
			m_log->error("ShowFilePage::handleRequest caught other exception");
		}	
	}

}

const bool ShowFilePage::WillHandleURI(const std::string &uri)
{
	if(uri.find("showfile.htm")!=std::string::npos || (m_filewhitelist.find(uri)!=m_filewhitelist.end()) || (uri.size()>1 && uri[0]=='/' && m_filewhitelist.find(uri.substr(1))!=m_filewhitelist.end()))
	{
		return true;
	}
	else
	{
		return false;
	}
}
