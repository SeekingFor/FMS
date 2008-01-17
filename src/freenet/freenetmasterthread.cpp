#include "../../include/freenet/freenetmasterthread.h"
#include "../../include/option.h"
#include "../../include/uuidgenerator.h"
#include "../../include/stringfunctions.h"
#include "../../include/freenet/unkeyedidcreator.h"
#include "../../include/freenet/identityinserter.h"
#include "../../include/freenet/identityrequester.h"
#include "../../include/freenet/introductionpuzzleinserter.h"
#include "../../include/freenet/identityintroductionrequester.h"
#include "../../include/freenet/introductionpuzzlerequester.h"
#include "../../include/freenet/introductionpuzzleremover.h"
#include "../../include/freenet/identityintroductioninserter.h"
#include "../../include/freenet/trustlistinserter.h"

#include <zthread/Thread.h>

#ifdef XMEM
	#include <xmem.h>
#endif

FreenetMasterThread::FreenetMasterThread()
{
	std::string fcpport;

	if(Option::instance()->Get("FCPHost",m_fcphost)==false)
	{
		m_fcphost="localhost";
		Option::instance()->Set("FCPHost",m_fcphost);
	}
	if(Option::instance()->Get("FCPPort",fcpport)==false)
	{
		fcpport="9481";
		Option::instance()->Set("FCPPort",fcpport);
	}

	// convert fcp port to long, and make sure it's within the valid port range
	if(StringFunctions::Convert(fcpport,m_fcpport)==false)
	{
		m_fcpport=9481;
		Option::instance()->Set("FCPPort","9481");
	}

	m_receivednodehello=false;

}

FreenetMasterThread::~FreenetMasterThread()
{

}

const bool FreenetMasterThread::FCPConnect()
{
	// we were previosly connected, send FCPDisconnect to objects
	if(m_receivednodehello==true)
	{
		for(std::vector<IFCPConnected *>::iterator i=m_fcpconnected.begin(); i!=m_fcpconnected.end(); i++)
		{
			(*i)->FCPDisconnected();
		}
		m_receivednodehello=false;
	}

	m_log->WriteLog(LogFile::LOGLEVEL_INFO,__FUNCTION__" trying to connect to node "+m_fcphost);

	if(m_fcp.Connect(m_fcphost.c_str(),m_fcpport)==true)
	{
		UUIDGenerator uuid;
		std::string clientname="FMSClient-"+uuid.Generate();
		// send ClientHello message to node
		m_fcp.SendMessage("ClientHello",2,"Name",clientname.c_str(),"ExpectedVersion","2.0");

		m_log->WriteLog(LogFile::LOGLEVEL_INFO,__FUNCTION__" connected to node");

		return true;
	}
	else
	{
		return false;
	}

}

const bool FreenetMasterThread::HandleMessage(FCPMessage &message)
{
	if(message.GetName()=="NodeHello")
	{
		m_receivednodehello=true;

		// send connected message to all objects, must do this AFTER we received the NodeHello message
		for(std::vector<IFCPConnected *>::iterator i=m_fcpconnected.begin(); i!=m_fcpconnected.end(); i++)
		{
			(*i)->FCPConnected();
		}

		return true;
	}
	if(m_receivednodehello==true)
	{
		bool handled=false;
		std::vector<IFCPMessageHandler *>::iterator i=m_fcpmessagehandlers.begin();
		while(handled==false && i!=m_fcpmessagehandlers.end())
		{
			handled=(*i)->HandleMessage(message);
			i++;
		}

		if(handled==false)
		{
			std::string info("");
			for(std::map<std::string,std::string>::iterator mi=message.begin(); mi!=message.end(); mi++)
			{
				info+="\t\t\t\t"+(*mi).first+"="+(*mi).second+"\r\n";
			}
			m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,__FUNCTION__" received unhandled "+message.GetName()+" message.  Message content :\r\n"+info);

			// if unhandled message was alldata - we must retrieve the data
			if(message.GetName()=="AllData")
			{
				long length;
				StringFunctions::Convert(message["DataLength"],length);
				while(m_fcp.Connected() && m_fcp.ReceiveBufferSize()<length)
				{
					m_fcp.Update(1);
				}
				if(m_fcp.Connected())
				{
					char *data=new char[length];
					m_fcp.ReceiveRaw(data,length);
					delete [] data;
				}
			}
		}

		return handled;

	}
	else
	{
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,__FUNCTION__" received "+message.GetName()+" message before NodeHello");
	}

	return false;
}

void FreenetMasterThread::RegisterFCPConnected(IFCPConnected *obj)
{
	m_fcpconnected.push_back(obj);
}

void FreenetMasterThread::RegisterFCPMessageHandler(IFCPMessageHandler *obj)
{
	m_fcpmessagehandlers.push_back(obj);
}

void FreenetMasterThread::RegisterPeriodicProcessor(IPeriodicProcessor *obj)
{
	m_processors.push_back(obj);
}

void FreenetMasterThread::run()
{

	FCPMessage message;
	bool done=false;

	Setup();

	do
	{
		if(m_fcp.Connected()==false)
		{
			if(FCPConnect()==false)
			{

				m_log->WriteLog(LogFile::LOGLEVEL_ERROR,__FUNCTION__" could not connect to node.  Waiting 60 seconds.");

				// wait 60 seconds - will then try to connect again
				try
				{
					ZThread::Thread::sleep(60000);
				}
				catch(...)
				{
					done=true;
				}
			}
		}
		// fcp is connected
		else
		{
			m_fcp.Update(1);

			// check for message on receive buffer and handle it
			if(m_fcp.ReceiveBufferSize()>0)
			{
				message.Reset();
				message=m_fcp.ReceiveMessage();

				if(message.GetName()!="")
				{
					HandleMessage(message);
				}
			}

			// let objects do their processing
			for(std::vector<IPeriodicProcessor *>::iterator i=m_processors.begin(); i!=m_processors.end(); i++)
			{
				(*i)->Process();
			}

		}
	}while(!ZThread::Thread::interrupted() && done==false);

	m_fcp.Disconnect();

	Shutdown();

}

void FreenetMasterThread::Setup()
{

	// seed random number generator
	srand(time(NULL));

	m_registrables.push_back(new UnkeyedIDCreator(&m_fcp));
	m_registrables.push_back(new IdentityInserter(&m_fcp));
	m_registrables.push_back(new IdentityRequester(&m_fcp));
	m_registrables.push_back(new IntroductionPuzzleInserter(&m_fcp));
	m_registrables.push_back(new IdentityIntroductionRequester(&m_fcp));
	m_registrables.push_back(new IntroductionPuzzleRequester(&m_fcp));
	m_registrables.push_back(new IntroductionPuzzleRemover());
	m_registrables.push_back(new IdentityIntroductionInserter(&m_fcp));
	m_registrables.push_back(new TrustListInserter(&m_fcp));

	for(std::vector<IFreenetRegistrable *>::iterator i=m_registrables.begin(); i!=m_registrables.end(); i++)
	{
		(*i)->RegisterWithThread(this);
	}

}

void FreenetMasterThread::Shutdown()
{
	// delete each registerable object
	for(std::vector<IFreenetRegistrable *>::iterator i=m_registrables.begin(); i!=m_registrables.end(); i++)
	{
		delete (*i);
	}
}