#include "../../include/http/httpthread.h"
#include "../../include/http/fmshttprequesthandlerfactory.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>

#include <iostream>

#ifdef XMEM
	#include <xmem.h>
#endif

HTTPThread::HTTPThread()
{
	m_listenport=8080;
}

void HTTPThread::run()
{
	m_log->debug("HTTPThread::run thread started.");

	LoadDatabase();
	Option option(m_db);

	std::string portstr("8080");
	option.Get("HTTPListenPort",portstr);
	StringFunctions::Convert(portstr,m_listenport);

	try
	{
		Poco::Net::ServerSocket sock(m_listenport);
		Poco::Net::HTTPServerParams* pParams = new Poco::Net::HTTPServerParams;
		pParams->setMaxQueued(30);
		pParams->setMaxThreads(5);
		Poco::Net::HTTPServer srv(new FMSHTTPRequestHandlerFactory(m_db),sock,pParams);

		srv.start();
		m_log->trace("Started HTTPServer");

		do
		{
			Poco::Thread::sleep(1000);
		}while(!IsCancelled());

		m_log->trace("Trying to stop HTTPServer");
		srv.stop();
		m_log->trace("Stopped HTTPServer");
		
		m_log->trace("Waiting for current HTTP requests to finish");
		while(srv.currentConnections()>0)
		{
			Poco::Thread::sleep(500);
		}
	}
	catch(Poco::Exception &e)
	{
		m_log->fatal("HTTPThread::run caught "+e.displayText());
	}
	catch(...)
	{
		m_log->fatal("HTTPThread::run caught unknown exception");
	}

	m_log->debug("HTTPThread::run thread exiting.");

}
