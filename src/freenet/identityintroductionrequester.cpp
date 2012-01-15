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

IdentityIntroductionRequester::IdentityIntroductionRequester(SQLite3DB::DB *db):IIndexRequester<std::string>(db)
{
	Initialize();
}

IdentityIntroductionRequester::IdentityIntroductionRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<std::string>(db,fcp)
{
	Initialize();
}

const std::string IdentityIntroductionRequester::GetIDFromIdentifier(const std::string &identifier)
{
	std::string UUID("");
	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	if(idparts.size()>=3)
	{
		StringFunctions::Convert(idparts[3],UUID);
	}
	return UUID;
}

const bool IdentityIntroductionRequester::HandleAllData(FCPv2::Message &message)
{
	FreenetSSKKey ssk;
	Poco::DateTime date;
	std::vector<char> data;
	long datalength;
	IdentityIntroductionXML xml;
	SQLite3DB::Transaction trans(m_db);
	const std::string UUID = GetIDFromIdentifier(message["Identifier"]);

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

	trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

	// parse file into xml and update the database
	if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{
		// mark puzzle found
		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET FoundSolution='true' WHERE UUID=?;");
		st.Bind(0,UUID);
		trans.Step(st);
		trans.Finalize(st);

		if(ssk.TryParse(xml.GetIdentity())==true)
		{
			// try to find existing identity with this SSK
			st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
			st.Bind(0,ssk.GetBaseKey());
			trans.Step(st);
			if(st.RowReturned()==false)
			{
				// we don't already know about this id - add it
				trans.Finalize(st);
				date=Poco::Timestamp();
				st=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod,SolvedPuzzleCount,IsFMS) VALUES(?,?,?,1,1);");
				st.Bind(0,ssk.GetBaseKey());
				st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
				st.Bind(2,"solved captcha");
				trans.Step(st);
			}
			else
			{
				int id=0;
				st.ResultInt(0,id);
				st=m_db->Prepare("UPDATE tblIdentity SET SolvedPuzzleCount=SolvedPuzzleCount+1, IsFMS=1 WHERE IdentityID=?;");
				st.Bind(0,id);
				trans.Step(st);
			}
			trans.Finalize(st);

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
		st.Bind(0,UUID);
		trans.Step(st);
		trans.Finalize(st);	

		m_log->error("IdentityIntroductionRequester::HandleAllData error parsing IdentityIntroduction XML file : "+message["Identifier"]);
	}

	trans.Commit();

	if(trans.IsSuccessful()==false)
	{
		m_log->error("IdentityIntroductionRequester::HandleAllData transaction failed with SQLite error:"+trans.GetLastErrorStr()+" SQL="+trans.GetErrorSQL());
	}

	// remove UUID from request list
	RemoveFromRequestList(UUID);

	return true;
}

const bool IdentityIntroductionRequester::HandleGetFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;

	StringFunctions::Split(message["Identifier"],"|",idparts);

	// fatal error - don't try to download again
	if(message["Fatal"]=="true")
	{
		if(message["Code"]!="25")
		{
			SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET FoundSolution='true' WHERE UUID=?;");
			st.Bind(0,idparts[3]);
			st.Step();
			st.Finalize();
		}

		m_log->debug("IdentityIntroductionRequester::HandleAllData Fatal GetFailed code="+message["Code"]+" for "+message["Identifier"]);
	}

	// remove UUID from request list
	RemoveFromRequestList(GetIDFromIdentifier(message["Identifier"]));

	return true;
}

void IdentityIntroductionRequester::Initialize()
{
	m_fcpuniquename="IdentityIntroductionRequester";
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
}

void IdentityIntroductionRequester::PopulateIDList()
{
	Poco::DateTime date;
	SQLite3DB::Transaction trans(m_db);

	date-=Poco::Timespan(1,0,0,0,0);

	m_ids.clear();

	// only selects, deferred OK
	trans.Begin();

	// get all non-solved puzzles from yesterday and today for this identity
	SQLite3DB::Statement st=m_db->Prepare("SELECT UUID FROM tblIntroductionPuzzleInserts WHERE Day>=? AND FoundSolution='false';");
	st.Bind(0, Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	trans.Step(st);

	while(st.RowReturned())
	{
		std::string uuid;
		st.ResultText(0,uuid);
		m_ids[uuid].m_requested=false;
		trans.Step(st);
	}

	trans.Finalize(st);
	trans.Commit();

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

		StartedRequest(UUID,message["Identifier"]);

	}

	m_ids[UUID].m_requested=true;

}
