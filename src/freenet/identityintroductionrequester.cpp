#include "../../include/freenet/identityintroductionrequester.h"
#include "../../include/freenet/identityintroductionxml.h"
#include "../../include/freenet/freenetkeys.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/hex.h"

#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/SHA1Engine.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityIntroductionRequester::IdentityIntroductionRequester(SQLite3DB::DB *db):IDatabase(db)
{
	Initialize();
}

IdentityIntroductionRequester::IdentityIntroductionRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IDatabase(db),IFCPConnected(fcp)
{
	Initialize();
}

void IdentityIntroductionRequester::FCPConnected()
{
	m_requesting.clear();
	PopulateIDList();
}

void IdentityIntroductionRequester::FCPDisconnected()
{
	
}

const bool IdentityIntroductionRequester::HandleAllData(FCPv2::Message &message)
{
	FreenetSSKKey ssk;
	Poco::DateTime date;
	std::vector<std::string> idparts;
	std::vector<char> data;
	long datalength;
	IdentityIntroductionXML xml;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);

	// wait for all data to be received from connection
	m_fcp->WaitForBytes(1000,datalength);

	// if we got disconnected- return immediately
	if(m_fcp->IsConnected()==false)
	{
		return false;
	}

	// receive the file
	m_fcp->Receive(data,datalength);

	// parse file into xml and update the database
	if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{
		// mark puzzle found
		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET FoundSolution='true' WHERE UUID=?;");
		st.Bind(0,idparts[3]);
		st.Step();
		st.Finalize();

		if(ssk.TryParse(xml.GetIdentity())==true)
		{
			// try to find existing identity with this SSK
			st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
			st.Bind(0,ssk.GetBaseKey());
			st.Step();
			if(st.RowReturned()==false)
			{
				// we don't already know about this id - add it
				st.Finalize();
				date=Poco::Timestamp();
				st=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod,SolvedPuzzleCount) VALUES(?,?,?,1);");
				st.Bind(0,ssk.GetBaseKey());
				st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
				st.Bind(2,"solved captcha");
				st.Step();
			}
			else
			{
				int id=0;
				st.ResultInt(0,id);
				st=m_db->Prepare("UPDATE tblIdentity SET SolvedPuzzleCount=SolvedPuzzleCount+1 WHERE IdentityID=?;");
				st.Bind(0,id);
				st.Step();
			}
			st.Finalize();

			m_log->debug("IdentityIntroductionRequester::HandleAddData parsed a valid identity.");
		}
		else
		{
			m_log->error("IdentityIntroductionRequester::HandleAllData parsed, public SSK key was not valid : "+xml.GetIdentity());
		}

		m_log->debug("IdentityIntroductionRequester::HandleAllData parsed IdentityIntroduction XML file : "+message["Identifier"]);
	}
	else
	{

		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET FoundSolution='true' WHERE UUID=?;");
		st.Bind(0,idparts[3]);
		st.Step();
		st.Finalize();		

		m_log->error("IdentityIntroductionRequester::HandleAllData error parsing IdentityIntroduction XML file : "+message["Identifier"]);
	}

	// remove UUID from request list
	RemoveFromRequestList(idparts[3]);

	return true;
}

const bool IdentityIntroductionRequester::HandleGetFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;

	StringFunctions::Split(message["Identifier"],"|",idparts);

	// fatal error - don't try to download again
	if(message["Fatal"]=="true")
	{
		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET FoundSolution='true' WHERE UUID=?;");
		st.Bind(0,idparts[3]);
		st.Step();
		st.Finalize();

		m_log->debug("IdentityIntroductionRequester::HandleAllData Fatal GetFailed for "+message["Identifier"]);
	}

	// remove UUID from request list
	RemoveFromRequestList(idparts[3]);

	return true;
}

const bool IdentityIntroductionRequester::HandleMessage(FCPv2::Message &message)
{

	if(message["Identifier"].find("IdentityIntroductionRequester")==0)
	{
		
		// ignore DataFound
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
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);
			RemoveFromRequestList(idparts[3]);
			return true;
		}

	}

	return false;
}

void IdentityIntroductionRequester::Initialize()
{
	m_maxrequests=0;
	Option option(m_db);
	option.GetInt("MaxIdentityIntroductionRequests",m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxIdentityIntroductionRequests is currently less than 1.  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxIdentityIntroductionRequests is currently set at more than 100.  This value might be incorrectly configured.");
	}
	option.Get("MessageBase",m_messagebase);
	m_tempdate=Poco::Timestamp();
}

void IdentityIntroductionRequester::PopulateIDList()
{
	Poco::DateTime date;
	int id;

	date-=Poco::Timespan(1,0,0,0,0);

	m_ids.clear();

	m_db->Execute("BEGIN;");

	// get all identities that have unsolved puzzles from yesterday or today
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblIntroductionPuzzleInserts WHERE Day>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND FoundSolution='false' GROUP BY LocalIdentityID;");
	st.Step();
	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}

	m_db->Execute("COMMIT;");

}

void IdentityIntroductionRequester::Process()
{
	// max is the smaller of the config value or the total number of identities we will request from
	long max=m_maxrequests>m_ids.size() ? m_ids.size() : m_maxrequests;

		// try to keep up to max requests going
	if(m_requesting.size()<max)
	{
		std::map<long,bool>::iterator i=m_ids.begin();
		while(i!=m_ids.end() && (*i).second==true)
		{
			i++;
		}

		if(i!=m_ids.end())
		{
			StartRequests((*i).first);
		}
		else
		{
			// we requested from all ids in the list, repopulate the list
			PopulateIDList();
		}
	}
	// special case - if there were 0 identities on the list when we started then we will never get a chance to repopulate the list
	// this will recheck for ids every minute
	Poco::DateTime now;
	if(m_ids.size()==0 && m_tempdate<(now-Poco::Timespan(0,0,1,0,0)))
	{
		PopulateIDList();
		m_tempdate=now;
	}
}

void IdentityIntroductionRequester::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void IdentityIntroductionRequester::RemoveFromRequestList(const std::string &UUID)
{
	std::vector<std::string>::iterator i=m_requesting.begin();
	while(i!=m_requesting.end() && (*i)!=UUID)
	{
		i++;
	}
	if(i!=m_requesting.end())
	{
		m_requesting.erase(i);
	}
}

void IdentityIntroductionRequester::StartRequest(const std::string &UUID)
{
	std::string day;
	std::string solution;
	std::string encodedhash;
	FCPv2::Message message;
	SQLite3DB::Statement st=m_db->Prepare("SELECT Day, PuzzleSolution FROM tblIntroductionPuzzleInserts WHERE FoundSolution='false' AND UUID=?;");
	st.Bind(0,UUID);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,day);
		st.ResultText(1,solution);

		// get the hash of the solution
		Poco::SHA1Engine sha1;
		sha1.update(solution);
		encodedhash=Poco::DigestEngine::digestToHex(sha1.digest());
		StringFunctions::UpperCase(encodedhash,encodedhash);

		//start request for the solution
		message.SetName("ClientGet");
		message["URI"]="KSK@"+m_messagebase+"|"+day+"|"+UUID+"|"+encodedhash+".xml";
		message["Identifier"]="IdentityIntroductionRequester|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="10000";

		m_fcp->Send(message);

		m_requesting.push_back(UUID);

	}

}

void IdentityIntroductionRequester::StartRequests(const long localidentityid)
{
	Poco::DateTime date;
	std::string localidentityidstr;
	std::string uuid;

	date-=Poco::Timespan(1,0,0,0,0);
	StringFunctions::Convert(localidentityid,localidentityidstr);

	// get all non-solved puzzles from yesterday and today for this identity
	SQLite3DB::Statement st=m_db->Prepare("SELECT UUID FROM tblIntroductionPuzzleInserts WHERE LocalIdentityID="+localidentityidstr+" AND Day>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND FoundSolution='false';");
	st.Step();

	// start requests for all non-solved puzzles
	while(st.RowReturned())
	{
		uuid="";
		st.ResultText(0,uuid);
		StartRequest(uuid);
		st.Step();
	}

	m_ids[localidentityid]=true;

}
