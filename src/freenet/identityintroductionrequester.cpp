#include "../../include/freenet/identityintroductionrequester.h"
#include "../../include/freenet/identityintroductionxml.h"
#include "../../include/freenet/freenetssk.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/xyssl/sha1.h"
#include "../../include/hex.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityIntroductionRequester::IdentityIntroductionRequester()
{
	Initialize();
}

IdentityIntroductionRequester::IdentityIntroductionRequester(FCPv2 *fcp):IFCPConnected(fcp)
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

const bool IdentityIntroductionRequester::HandleAllData(FCPMessage &message)
{
	FreenetSSK ssk;
	DateTime date;
	std::vector<std::string> idparts;
	std::vector<char> data;
	long datalength;
	IdentityIntroductionXML xml;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);

	// wait for all data to be received from connection
	while(m_fcp->Connected() && m_fcp->ReceiveBufferSize()<datalength)
	{
		m_fcp->Update(1);
	}

	// if we got disconnected- return immediately
	if(m_fcp->Connected()==false)
	{
		return false;
	}

	// receive the file
	data.resize(datalength);
	m_fcp->ReceiveRaw(&data[0],datalength);

	// parse file into xml and update the database
	if(xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{

		ssk.SetPublicKey(xml.GetIdentity());

		// mark puzzle found
		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET FoundSolution='true' WHERE UUID=?;");
		st.Bind(0,idparts[3]);
		st.Step();
		st.Finalize();

		if(ssk.ValidPublicKey()==true)
		{
			// try to find existing identity with this SSK
			st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
			st.Bind(0,xml.GetIdentity());
			st.Step();
			if(st.RowReturned()==false)
			{
				// we don't already know about this id - add it
				st.Finalize();
				date.SetToGMTime();
				st=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded) VALUES(?,?);");
				st.Bind(0,xml.GetIdentity());
				st.Bind(1,date.Format("%Y-%m-%d %H:%M:%S"));
				st.Step();
			}
			st.Finalize();

			m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IdentityIntroductionRequester::HandleAddData parsed a valid identity.");
		}
		else
		{
			m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"IdentityIntroductionRequester::HandleAllData parsed, public SSK key was not valid.");
		}

		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IdentityIntroductionRequester::HandleAllData parsed IdentityIntroduction XML file : "+message["Identifier"]);
	}
	else
	{

		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET FoundSolution='true' WHERE UUID=?;");
		st.Bind(0,idparts[3]);
		st.Step();
		st.Finalize();		

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"IdentityIntroductionRequester::HandleAllData error parsing IdentityIntroduction XML file : "+message["Identifier"]);
	}

	// remove UUID from request list
	RemoveFromRequestList(idparts[3]);

	return true;
}

const bool IdentityIntroductionRequester::HandleGetFailed(FCPMessage &message)
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

		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IdentityIntroductionRequester::HandleAllData Fatal GetFailed for "+message["Identifier"]);
	}

	// remove UUID from request list
	RemoveFromRequestList(idparts[3]);

	return true;
}

const bool IdentityIntroductionRequester::HandleMessage(FCPMessage &message)
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
	std::string tempval="";
	Option::instance()->Get("MaxIdentityIntroductionRequests",tempval);
	StringFunctions::Convert(tempval,m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"Option MaxIdentityIntroductionRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->WriteLog(LogFile::LOGLEVEL_WARNING,"Option MaxIdentityIntroductionRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
	Option::instance()->Get("MessageBase",m_messagebase);
	m_tempdate.SetToGMTime();
}

void IdentityIntroductionRequester::PopulateIDList()
{
	DateTime date;
	int id;

	date.SetToGMTime();
	date.Add(0,0,0,-1);

	// get all identities that have unsolved puzzles from yesterday or today
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblIntroductionPuzzleInserts WHERE Day>='"+date.Format("%Y-%m-%d")+"' AND FoundSolution='false' GROUP BY LocalIdentityID;");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}

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
	DateTime now;
	now.SetToGMTime();
	if(m_tempdate<(now-(1.0/1440.0)))
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
	std::vector<unsigned char> solutionhash;
	std::string encodedhash;
	FCPMessage message;
	SQLite3DB::Statement st=m_db->Prepare("SELECT Day, PuzzleSolution FROM tblIntroductionPuzzleInserts WHERE FoundSolution='false' AND UUID=?;");
	st.Bind(0,UUID);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,day);
		st.ResultText(1,solution);

		// get the hash of the solution
		solutionhash.resize(20);
		sha1((unsigned char *)solution.c_str(),solution.size(),&solutionhash[0]);
		Hex::Encode(solutionhash,encodedhash);

		//start request for the solution
		message.SetName("ClientGet");
		message["URI"]="KSK@"+m_messagebase+"|"+day+"|"+UUID+"|"+encodedhash+".xml";
		message["Identifier"]="IdentityIntroductionRequester|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="10000";

		m_fcp->SendMessage(message);

		m_requesting.push_back(UUID);

	}

}

void IdentityIntroductionRequester::StartRequests(const long localidentityid)
{
	DateTime date;
	std::string localidentityidstr;
	std::string uuid;

	date.SetToGMTime();
	date.Add(0,0,0,-1);
	StringFunctions::Convert(localidentityid,localidentityidstr);

	// get all non-solved puzzles from yesterday and today for this identity
	SQLite3DB::Statement st=m_db->Prepare("SELECT UUID FROM tblIntroductionPuzzleInserts WHERE LocalIdentityID="+localidentityidstr+" AND Day>='"+date.Format("%Y-%m-%d")+"' AND FoundSolution='false';");
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
