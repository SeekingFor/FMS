#include "../../include/freenet/freenetmasterthread.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/freenet/unkeyedidcreator.h"
#include "../../include/freenet/identityinserter.h"
#include "../../include/freenet/identityrequester.h"
#include "../../include/freenet/introductionpuzzleinserter.h"
#include "../../include/freenet/identityintroductionrequester.h"
#include "../../include/freenet/introductionpuzzlerequester.h"
#include "../../include/freenet/identityintroductioninserter.h"
#include "../../include/freenet/trustlistinserter.h"
#include "../../include/freenet/trustlistrequester.h"
#include "../../include/freenet/messagelistrequester.h"
#include "../../include/freenet/messagelistinserter.h"
#include "../../include/freenet/messagerequester.h"
#include "../../include/freenet/messageinserter.h"
#include "../../include/freenet/boardlistinserter.h"
#include "../../include/freenet/boardlistrequester.h"
#include "../../include/freenet/siteinserter.h"
#include "../../include/freenet/fileinserter.h"
#include "../../include/freenet/fmsversionrequester.h"

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/DateTime.h>
#include <Poco/Timespan.h>
#include <Poco/Thread.h>

#include <cstdlib>

#ifdef XMEM
	#include <xmem.h>
#endif

FreenetMasterThread::FreenetMasterThread()
{
	std::string fcpport;

	if(Option::Instance()->Get("FCPHost",m_fcphost)==false)
	{
		m_fcphost="localhost";
		Option::Instance()->Set("FCPHost",m_fcphost);
	}
	if(Option::Instance()->Get("FCPPort",fcpport)==false)
	{
		fcpport="9481";
		Option::Instance()->Set("FCPPort",fcpport);
	}

	// convert fcp port to long, and make sure it's within the valid port range
	if(StringFunctions::Convert(fcpport,m_fcpport)==false)
	{
		m_fcpport=9481;
		Option::Instance()->Set("FCPPort","9481");
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

	m_log->information("FreenetMasterThread::FCPConnect trying to connect to node "+m_fcphost);

	if(m_fcp.Connect(m_fcphost.c_str(),m_fcpport)==true)
	{
		Poco::UUIDGenerator uuidgen;
		Poco::UUID uuid;

		try
		{
			uuid=uuidgen.createRandom();
		}
		catch(...)
		{
			m_log->fatal("FreenetMasterThread::FCPConnect could not generate UUID");
		}

		std::string clientname="FMSClient-"+uuid.toString();
		// send ClientHello message to node
		m_fcp.SendMessage("ClientHello",2,"Name",clientname.c_str(),"ExpectedVersion","2.0");

		m_log->information("FreenetMasterThread::FCPConnect connected to node");

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
			m_log->debug("FreenetMasterThread::HandleMessage received unhandled "+message.GetName()+" message.  Message content :\r\n"+info);

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
		m_log->error("FreenetMasterThread::HandleMessage received "+message.GetName()+" message before NodeHello");
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

	Poco::DateTime lastreceivedmessage;
	Poco::DateTime lastconnected;
	Poco::DateTime now;
	FCPMessage message;
	bool done=false;

	lastconnected-=Poco::Timespan(0,0,1,0,0);

	m_log->debug("FreenetMasterThread::run thread started.");

	Setup();

	do
	{
		try
		{
			if(m_fcp.Connected()==false)
			{
				// wait at least 1 minute since last successful connect
				now=Poco::Timestamp();
				if(lastconnected<=(now-Poco::Timespan(0,0,1,0,0)))
				{
					if(FCPConnect()==false)
					{

						m_log->error("FreenetMasterThread::run could not connect to node.  Waiting 60 seconds.");

						for(int i=0; i<60 && !IsCancelled(); i++)
						{
							Poco::Thread::sleep(1000);
						}
					}
					else
					{
						lastreceivedmessage=Poco::Timestamp();
						lastconnected=Poco::Timestamp();
					}
				}
				else
				{
					Poco::Thread::sleep(1000);
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
						lastreceivedmessage=Poco::Timestamp();
					}
				}

				// let objects do their processing
				for(std::vector<IPeriodicProcessor *>::iterator i=m_processors.begin(); i!=m_processors.end(); i++)
				{
					(*i)->Process();
				}

				// if we haven't received any messages from the node in 10 minutes, something is wrong
				now=Poco::Timestamp();
				if(lastreceivedmessage<(now-Poco::Timespan(0,0,10,0,0)))
				{
					m_log->error("FreenetMasterThread::Run The Freenet node has not responded in 10 minutes.  Trying to reconnect.");
					m_fcp.Disconnect();
				}

				if(m_fcp.Connected()==false)
				{
					m_log->information("FreenetMasterThread::Run Disconnected from Freenet node.");
				}

			}
		}
		catch(Poco::Exception &e)
		{
			m_log->error("FreenetMasterThread::run caught exception : "+e.displayText());
		}
		catch(...)
		{
			m_log->error("FreenetMasterThread::run caught unknown exception");
		}
	}while(!IsCancelled() && done==false);

	m_fcp.Disconnect();

	Shutdown();

	m_log->debug("FreenetMasterThread::run thread exiting.");

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
	m_registrables.push_back(new IdentityIntroductionInserter(&m_fcp));
	m_registrables.push_back(new TrustListInserter(&m_fcp));
	m_registrables.push_back(new TrustListRequester(&m_fcp));
	m_registrables.push_back(new MessageListInserter(&m_fcp));
	m_registrables.push_back(new MessageListRequester(&m_fcp));
	m_registrables.push_back(new MessageInserter(&m_fcp));
	m_registrables.push_back(new MessageRequester(&m_fcp));
	m_registrables.push_back(new BoardListInserter(&m_fcp));
	m_registrables.push_back(new BoardListRequester(&m_fcp));
	m_registrables.push_back(new SiteInserter(&m_fcp));
	m_registrables.push_back(new FileInserter(&m_fcp));
	m_registrables.push_back(new FMSVersionRequester(&m_fcp));

	for(std::vector<IFreenetRegistrable *>::iterator i=m_registrables.begin(); i!=m_registrables.end(); i++)
	{
		(*i)->RegisterWithThread(this);
	}

}

void FreenetMasterThread::Shutdown()
{
	// delete each registrable object
	for(std::vector<IFreenetRegistrable *>::iterator i=m_registrables.begin(); i!=m_registrables.end(); i++)
	{
		delete (*i);
	}
}
