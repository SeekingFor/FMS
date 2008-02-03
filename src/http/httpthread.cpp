#include "../../include/http/httpthread.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/http/pages/homepage.h"
#include "../../include/http/pages/optionspage.h"
#include "../../include/http/pages/showcaptchapage.h"
#include "../../include/http/pages/createidentitypage.h"
#include "../../include/http/pages/localidentitiespage.h"
#include "../../include/http/pages/announceidentitypage.h"
#include "../../include/http/pages/addpeerpage.h"
#include "../../include/http/pages/peertrustpage.h"

#include <iostream>

#ifdef XMEM
	#include <xmem.h>
#endif

HTTPThread::HTTPThread()
{
	std::string templatestr;
	int port;
	std::string portstr;
	Option::Instance()->Get("HTTPListenPort",portstr);
	StringFunctions::Convert(portstr,port);

	// set template
	templatestr="<html><head></head><body><a href=\"home.htm\">Home</a><br>[CONTENT]</body></html>";
	FILE *infile=fopen("template.htm","r+b");
	if(infile)
	{
		fseek(infile,0,SEEK_END);
		long len=ftell(infile);
		std::vector<char> data(len,0);
		fseek(infile,0,SEEK_SET);
		fread(&data[0],1,len,infile);
		fclose(infile);
		templatestr.assign(data.begin(),data.end());
	}
	else
	{
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"HTTPThread::HTTPThread could not open template.htm");
	}

	// push back page handlers
	m_pagehandlers.push_back(new ShowCaptchaPage());
	m_pagehandlers.push_back(new OptionsPage(templatestr));
	m_pagehandlers.push_back(new LocalIdentitiesPage(templatestr));
	m_pagehandlers.push_back(new CreateIdentityPage(templatestr));
	m_pagehandlers.push_back(new AnnounceIdentityPage(templatestr));
	m_pagehandlers.push_back(new AddPeerPage(templatestr));
	m_pagehandlers.push_back(new PeerTrustPage(templatestr));
	// homepage must be last - catch all page handler
	m_pagehandlers.push_back(new HomePage(templatestr));

	m_ctx=0;
	m_ctx=shttpd_init(NULL,"listen_ports",portstr.c_str(),NULL);
	shttpd_listen(m_ctx,port,false);

	shttpd_register_uri(m_ctx,"*",HTTPThread::PageCallback,this);

}

HTTPThread::~HTTPThread()
{
	shttpd_fini(m_ctx);

	for(std::vector<IPageHandler *>::iterator i=m_pagehandlers.begin(); i!=m_pagehandlers.end(); i++)
	{
		delete (*i);
	}

}

void HTTPThread::PageCallback(shttpd_arg *arg)
{

	HTTPThread *thread=(HTTPThread *)arg->user_data;

	for(std::vector<IPageHandler *>::iterator i=thread->m_pagehandlers.begin(); i!=thread->m_pagehandlers.end(); )
	{
		if((*i)->Handle(arg)==true)
		{
			i=thread->m_pagehandlers.end();
		}
		else
		{
			i++;
		}
	}

}

void HTTPThread::Run()
{
	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"HTTPThread::run thread started.");

	do
	{
		shttpd_poll(m_ctx,1000);
	}while(!IsCancelled());

	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"HTTPThread::run thread exiting.");

}
