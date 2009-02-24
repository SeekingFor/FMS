#include "../../../include/http/pages/showimagepage.h"

#include <cstdio>

#ifdef XMEM
	#include <xmem.h>
#endif

std::map<std::string,std::vector<char> > ShowImagePage::m_imagecache;
std::set<std::string> ShowImagePage::m_imagewhitelist;

ShowImagePage::ShowImagePage(SQLite3DB::DB *db):IPageHandler(db)
{
	m_imagewhitelist.insert("images/new_posts.png");
	m_imagewhitelist.insert("images/no_new_posts.png");
}

void ShowImagePage::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("ShowImagePage::handleRequest from "+request.clientAddress().toString());

	std::map<std::string,std::string> queryvars;
	CreateQueryVarMap(request,queryvars);

	if(request.getVersion()==Poco::Net::HTTPRequest::HTTP_1_1)
	{
		response.setChunkedTransferEncoding(true);
	}

	std::string content="";
	if(queryvars.find("image")!=queryvars.end() && m_imagewhitelist.find((*queryvars.find("image")).second)!=m_imagewhitelist.end())
	{
		if(m_imagecache.find((*queryvars.find("image")).second)!=m_imagecache.end())
		{
			content+=std::string(m_imagecache[(*queryvars.find("image")).second].begin(),m_imagecache[(*queryvars.find("image")).second].end());
		}
		else
		{
			FILE *infile=fopen((*queryvars.find("image")).second.c_str(),"rb");
			if(infile)
			{
				fseek(infile,0,SEEK_END);
				long filelen=ftell(infile);
				fseek(infile,0,SEEK_SET);

				if(filelen>0)
				{
					std::vector<char> data(filelen,0);
					fread(&data[0],1,data.size(),infile);
					content+=std::string(data.begin(),data.end());
					m_imagecache[(*queryvars.find("image")).second]=data;
				}

				fclose(infile);
			}
		}
	}

	std::ostream &ostr = response.send();
	ostr << content;
}

const bool ShowImagePage::WillHandleURI(const std::string &uri)
{
	if(uri.find("showimage.htm")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
