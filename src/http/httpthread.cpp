#include "../../include/http/httpthread.h"
#include "../../include/http/fmshttprequesthandlerfactory.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/fmsapp.h"

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

	LoadDatabase(m_log);
	Option option(m_db);

	m_db->Execute("CREATE TEMPORARY TABLE IF NOT EXISTS tmpForumViewState(\
				  ForumViewStateID	TEXT NOT NULL UNIQUE,\
				  LocalIdentityID	INTEGER,\
				  BoardID			INTEGER,\
				  Page				INTEGER,\
				  ThreadID			INTEGER,\
				  MessageID			INTEGER,\
				  ReplyToMessageID	INTEGER\
				  );");

	m_db->Execute("CREATE TEMPORARY TABLE IF NOT EXISTS tmpFileAttachment(\
				  FileAttachmentID	INTEGER PRIMARY KEY,\
				  DateUploaded		DATETIME,\
				  ForumViewStateID	TEXT,\
				  FileName			TEXT,\
				  Data				BLOB,\
				  DataLength		INTEGER,\
				  ContentType		TEXT,\
				  FreenetKey		TEXT\
				  );");

	std::string bindaddress("0.0.0.0");
	option.GetInt("HTTPListenPort",m_listenport);
	option.Get("HTTPBindAddress",bindaddress);

	global::httplistenport=m_listenport;

	try
	{
		
		Poco::Net::SocketAddress sa(bindaddress,m_listenport);
		Poco::Net::ServerSocket sock(sa);
		//Poco::Net::ServerSocket sock(m_listenport);
		Poco::Net::HTTPServerParams* pParams = new Poco::Net::HTTPServerParams;
		//pParams->setMaxQueued(30);
		//pParams->setMaxThreads(5);
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
	catch(SQLite3DB::Exception &e)
	{
		m_log->fatal("HTTPThread caught SQLite3DB::Exception "+e.what());
		((FMSApp *)&FMSApp::instance())->Terminate();
	}
	catch(...)
	{
		m_log->fatal("HTTPThread::run caught unknown exception");
	}

	m_log->debug("HTTPThread::run thread exiting.");

}
