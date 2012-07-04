#ifndef _iindexrequester_
#define _iindexrequester_

#include "../idatabase.h"
#include "../ilogger.h"
#include "../option.h"
#include "../stringfunctions.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>

#include <algorithm>

#ifdef XMEM
	#include <xmem.h>
#endif

template <class IDTYPE>
class IIndexRequester:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IPeriodicProcessor,public IDatabase,public ILogger
{
public:
	IIndexRequester(SQLite3DB::DB *db);
	IIndexRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);
	virtual ~IIndexRequester()		{}

	virtual void FCPConnected();
	virtual void FCPDisconnected();
	virtual const bool HandleMessage(FCPv2::Message &message);

	virtual void Process();

	virtual void RegisterWithThread(FreenetMasterThread *thread);

protected:

	struct idstruct
	{
		idstruct():m_requested(false),m_fcpidentifier(""),m_flag(false)	{ }

		bool m_requested;
		bool m_flag;		// derived class can use flag for its own purposes
		std::string m_fcpidentifier;
	};

	void InitializeIIndexRequester();
	virtual void Initialize()=0;		// initialize m_maxrequests and m_fcpuniquename
	virtual void PopulateIDList()=0;
	virtual void StartRequest(const IDTYPE &id)=0;
	virtual const bool HandleAllData(FCPv2::Message &message)=0;
	virtual const bool HandleGetFailed(FCPv2::Message &message)=0;
	virtual const IDTYPE GetIDFromIdentifier(const std::string &identifier)=0;
	virtual void RemoveFromRequestList(const IDTYPE id);
	void RemoveFCPRequest(const std::string &identifier);
	void StartedRequest(const IDTYPE &id, const std::string &identifier);

	Poco::DateTime m_tempdate;
	Poco::DateTime m_lastreceived;
	Poco::DateTime m_lastpopulated;
	std::string m_messagebase;
	bool m_reverserequest;						// start from the back of the map and work forward
	std::map<IDTYPE,idstruct> m_ids;			// map of all ids we know and whether we have requested file from them yet
	std::string m_defaultrequestpriorityclassstr;

	// these MUST be populated by child class
	int m_maxrequests;
	std::string m_fcpuniquename;

private:
	std::vector<IDTYPE> m_requesting;		// list of ids we are currently requesting from

};

template <class IDTYPE>
IIndexRequester<IDTYPE>::IIndexRequester(SQLite3DB::DB *db):IDatabase(db)
{
	InitializeIIndexRequester();
}

template <class IDTYPE>
IIndexRequester<IDTYPE>::IIndexRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IDatabase(db),IFCPConnected(fcp)
{
	InitializeIIndexRequester();
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::FCPConnected()
{
	// make sure variables have been initialized by the derived class
	if(m_maxrequests==-1)
	{
		m_log->fatal(m_fcpuniquename+"::FCPConnected maxrequests not initialized correctly!");
	}
	if(m_fcpuniquename=="")
	{
		m_log->fatal(m_fcpuniquename+"::FCPConnected fcpuniquename not initialized correctly!");
	}
	if(m_fcpuniquename.find('|')!=std::string::npos)
	{
		m_log->fatal(m_fcpuniquename+"::FCPConnected fcpuniquename "+m_fcpuniquename+" contains | character!  This is not a valid character!");
		StringFunctions::Replace(m_fcpuniquename,"|","_");
	}

	m_lastreceived=Poco::Timestamp();
	m_requesting.clear();
	if(m_maxrequests>0)
	{
		PopulateIDList();
	}
	m_lastpopulated=Poco::Timestamp();
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::FCPDisconnected()
{
	
}

template <class IDTYPE>
const bool IIndexRequester<IDTYPE>::HandleMessage(FCPv2::Message &message)
{

	if(message["Identifier"].find(m_fcpuniquename)==0)
	{

		m_lastreceived=Poco::Timestamp();

		if(message.GetName()=="DataFound")
		{
			return true;
		}

		if(message.GetName()=="AllData")
		{
			return HandleAllData(message);
		}

		if(message.GetName()=="GetFailed")
		{
			return HandleGetFailed(message);
		}

		if(message.GetName()=="IdentifierCollision")
		{
			// remove one of the ids from the requesting list
			IDTYPE id=GetIDFromIdentifier(message["Identifier"]);
			/*
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);
			StringFunctions::Convert(idparts[1],id);
			*/
			RemoveFromRequestList(id);
			return true;
		}

		if(message.GetName()=="PersistentRequestRemoved")
		{
			m_log->trace(m_fcpuniquename+"::HandleMessage handled PersistentRequestRemoved for "+message["Identifier"]);
		}
	}

	return false;
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::InitializeIIndexRequester()
{
	m_maxrequests=-1;
	m_fcpuniquename="";
	m_reverserequest=false;
	Option option(m_db);

	option.Get("MessageBase",m_messagebase);
	m_tempdate=Poco::Timestamp();
	m_lastreceived=Poco::Timestamp();
	m_lastpopulated=Poco::Timestamp();
	m_lastpopulated-=Poco::Timespan(0,0,10,0,0);
	option.Get("DefaultRequestPriorityClass",m_defaultrequestpriorityclassstr);
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::Process()
{
	Poco::DateTime now;

	// max is the smaller of the config value or the total number of ids we will request from
	typename std::map<IDTYPE,bool>::size_type max=m_maxrequests>m_ids.size() ? m_ids.size() : m_maxrequests;

	// try to keep up to max requests going
	if(m_requesting.size()<max)
	{
		if(m_reverserequest==false)
		{
			typename std::map<IDTYPE,idstruct>::iterator i=m_ids.begin();

			while(i!=m_ids.end() && (*i).second.m_requested==true)
			{
				i++;
			}

			if(i!=m_ids.end())
			{
				StartRequest((*i).first);
			}
			else
			{
				// we requested from all ids in the list, repopulate the list (only every 10 minutes)
				if(m_lastpopulated<(now-Poco::Timespan(0,0,10,0,0)))
				{
					if(m_maxrequests>0)
					{
						PopulateIDList();
					}
					m_lastpopulated=Poco::Timestamp();
				}
			}
		}
		else
		{
			typename std::map<IDTYPE,idstruct>::reverse_iterator i=m_ids.rbegin();

			while(i!=m_ids.rend() && (*i).second.m_requested==true)
			{
				i++;
			}

			if(i!=m_ids.rend())
			{
				StartRequest((*i).first);
			}
			else
			{
				// we requested from all ids in the list, repopulate the list (only every 10 minutes)
				if(m_lastpopulated<(now-Poco::Timespan(0,0,10,0,0)))
				{
					if(m_maxrequests>0)
					{
						PopulateIDList();
					}
					m_lastpopulated=Poco::Timestamp();
				}
			}
		}
	}
	// special case - if there were 0 ids on the list when we started then we will never get a chance to repopulate the list
	// this will recheck for ids every minute
	if(m_ids.size()==0 && m_tempdate<(now-Poco::Timespan(0,0,1,0,0)))
	{
		if(m_maxrequests>0)
		{
			PopulateIDList();
		}
		m_tempdate=now;
		m_lastreceived=now;
	}
	// if we haven't received any messages to this object in 10 minutes, clear the requests and repopulate id list
	if(m_ids.size()>0 && m_lastreceived<(now-Poco::Timespan(0,0,10,0,0)))
	{
		m_log->error(m_fcpuniquename+"::Process has not received any messages in 10 minutes.  Restarting requests.");
		for(typename std::vector<IDTYPE>::const_iterator i=m_requesting.begin(); i!=m_requesting.end(); i++)
		{
			RemoveFCPRequest(m_ids[(*i)].m_fcpidentifier);
		}
		FCPConnected();
	}

}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::RemoveFCPRequest(const std::string &identifier)
{
	FCPv2::Message message("RemoveRequest");
	message["Identifier"]=identifier;
	message["Global"]="false";

	m_fcp->Send(message);

	m_log->trace(m_fcpuniquename+"::RemoveFCPRequest removing "+identifier);
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::RemoveFromRequestList(const IDTYPE id)
{
	typename std::vector<IDTYPE>::iterator i=std::find(m_requesting.begin(),m_requesting.end(),id);

	if(i!=m_requesting.end())
	{
		m_requesting.erase(i);
	}
	else
	{
		//m_log->trace("IIndexRequester<IDTYPE>::RemoveFromRequestList no matching id found!");
	}
}

template <class IDTYPE>
void IIndexRequester<IDTYPE>::StartedRequest(const IDTYPE &id, const std::string &identifier)
{
	m_requesting.push_back(id);
	m_ids[id].m_fcpidentifier=identifier;
}

#endif	// _iindexrequester_
