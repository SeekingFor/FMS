#include "../../include/http/fmshttprequesthandlerfactory.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/http/pages/homepage.h"
#include "../../include/http/pages/optionspage.h"
#include "../../include/http/pages/createidentitypage.h"
#include "../../include/http/pages/localidentitiespage.h"
#include "../../include/http/pages/confirmpage.h"
#include "../../include/http/pages/showcaptchapage.h"
#include "../../include/http/pages/announceidentitypage.h"
#include "../../include/http/pages/execquerypage.h"
#include "../../include/http/pages/boardspage.h"
#include "../../include/http/pages/insertedfilespage.h"
#include "../../include/http/pages/addpeerpage.h"
#include "../../include/http/pages/peerdetailspage.h"
#include "../../include/http/pages/controlboardpage.h"
#include "../../include/http/pages/peermaintenancepage.h"
#include "../../include/http/pages/peertrustpage.h"
#include "../../include/http/pages/versioninfopage.h"

FMSHTTPRequestHandlerFactory::FMSHTTPRequestHandlerFactory()
{
	// set template
	std::string templatestr="<html><head></head><body><a href=\"home.htm\">Home</a><br><h1>Could not open template.htm!  Place in program directory and restart!</h1><br>[CONTENT]</body></html>";
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
		m_log->error("HTTPThread::HTTPThread could not open template.htm");
	}

	// push back page handlers
	m_pagehandlers.push_back(new OptionsPage(templatestr));
	m_pagehandlers.push_back(new CreateIdentityPage(templatestr));
	m_pagehandlers.push_back(new LocalIdentitiesPage(templatestr));
	m_pagehandlers.push_back(new ConfirmPage(templatestr));
	m_pagehandlers.push_back(new ShowCaptchaPage());
	m_pagehandlers.push_back(new AnnounceIdentityPage(templatestr));
	m_pagehandlers.push_back(new ExecQueryPage(templatestr));
	m_pagehandlers.push_back(new BoardsPage(templatestr));
	m_pagehandlers.push_back(new InsertedFilesPage(templatestr));
	m_pagehandlers.push_back(new AddPeerPage(templatestr));
	m_pagehandlers.push_back(new PeerDetailsPage(templatestr));
	m_pagehandlers.push_back(new ControlBoardPage(templatestr));
	m_pagehandlers.push_back(new PeerMaintenancePage(templatestr));
	m_pagehandlers.push_back(new PeerTrustPage(templatestr));
	m_pagehandlers.push_back(new VersionInfoPage(templatestr));
	// homepage must be last - catch all page handler
	m_pagehandlers.push_back(new HomePage(templatestr));

	// initialize the access control list
	std::string aclstr;
	std::vector<std::string> aclparts;
	Option::Instance()->Get("HTTPAccessControl",aclstr);
	StringFunctions::Split(aclstr,",",aclparts);
	for(std::vector<std::string>::iterator i=aclparts.begin(); i!=aclparts.end(); i++)
	{
		m_acl.Add((*i));
	}
}

FMSHTTPRequestHandlerFactory::~FMSHTTPRequestHandlerFactory()
{

	for(std::vector<IPageHandler *>::iterator i=m_pagehandlers.begin(); i!=m_pagehandlers.end(); i++)
	{
		delete (*i);
	}

}

Poco::Net::HTTPRequestHandler *FMSHTTPRequestHandlerFactory::createRequestHandler(const Poco::Net::HTTPServerRequest &request)
{
	if(m_acl.IsAllowed(request.clientAddress().host()))
	{
		for(std::vector<IPageHandler *>::iterator i=m_pagehandlers.begin(); i!=m_pagehandlers.end(); i++)
		{
			if((*i)->WillHandleURI(request.getURI()))
			{
				// we need to return a new object because the HTTPServer will destory it when it's done.
				return (*i)->New();
			}
		}
	}
	else
	{
		m_log->debug("FMSHTTPRequestHandlerFactory::createRequestHandler host denied access "+request.clientAddress().host().toString());
	}
	return 0;
}
