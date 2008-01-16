#include "../../include/freenet/introductionpuzzlerequester.h"
#include "../../include/freenet/introductionpuzzlexml.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IntroductionPuzzleRequester::IntroductionPuzzleRequester()
{
	Initialize();
}

IntroductionPuzzleRequester::IntroductionPuzzleRequester(FCPv2 *fcp):IFCPConnected(fcp)
{
	Initialize();
}

void IntroductionPuzzleRequester::FCPConnected()
{
	m_requesting.clear();
	PopulateIDList();
}

void IntroductionPuzzleRequester::FCPDisconnected()
{
	
}

const bool IntroductionPuzzleRequester::HandleAllData(FCPMessage &message)
{
	DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	IntroductionPuzzleXML xml;
	long identityid;
	long index;

	now.SetToGMTime();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);

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
		st=m_db->Prepare("INSERT INTO tblIntroductionPuzzleRequests(IdentityID,Day,RequestIndex,Found,UUID,Type,MimeType,PuzzleData) VALUES(?,?,?,?,?,?,?,?);");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Bind(3,"true");
		st.Bind(4,xml.GetUUID());
		st.Bind(5,xml.GetType());
		st.Bind(6,xml.GetMimeType());
		st.Bind(7,xml.GetPuzzleData());
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,__FUNCTION__" parsed IntroductionPuzzle XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblIntroductionPuzzleRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,__FUNCTION__" error parsing IntroductionPuzzle XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool IntroductionPuzzleRequester::HandleGetFailed(FCPMessage &message)
{
	DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long identityid;
	long index;

	now.SetToGMTime();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);	

	// if this is a fatal error - insert index into database so we won't try to download this index again
	if(message["Fatal"]=="true")
	{
		st=m_db->Prepare("INSERT INTO tblIntroductionPuzzleRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,__FUNCTION__" fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool IntroductionPuzzleRequester::HandleMessage(FCPMessage &message)
{

	if(message["Identifier"].find("IntroductionPuzzleRequester")==0)
	{
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
			long identityid=0;
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);
			StringFunctions::Convert(idparts[1],identityid);
			RemoveFromRequestList(identityid);
			return true;
		}
	}

	return false;
}

void IntroductionPuzzleRequester::Initialize()
{
	std::string tempval="";
	Option::instance()->Get("MaxIntroductionPuzzleRequests",tempval);
	StringFunctions::Convert(tempval,m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"Option MaxIntroductionPuzzleRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->WriteLog(LogFile::LOGLEVEL_WARNING,"Option MaxIntroductionPuzzleRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
	Option::instance()->Get("MessageBase",m_messagebase);
	m_tempdate.SetToGMTime();
}

void IntroductionPuzzleRequester::PopulateIDList()
{
	DateTime now;
	int id;

	now.SetToGMTime();

	// select identities that aren't single use and have been seen today
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND SingleUse='false' AND LastSeen>='"+now.Format("%Y-%m-%d")+"';");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}

void IntroductionPuzzleRequester::Process()
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
			StartRequest((*i).first);
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

void IntroductionPuzzleRequester::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void IntroductionPuzzleRequester::RemoveFromRequestList(const long identityid)
{
	std::vector<long>::iterator i=m_requesting.begin();
	while(i!=m_requesting.end() && (*i)!=identityid)
	{
		i++;
	}
	if(i!=m_requesting.end())
	{
		m_requesting.erase(i);
	}
}

void IntroductionPuzzleRequester::StartRequest(const long identityid)
{
	DateTime now;
	FCPMessage message;
	std::string publickey;
	int index;
	std::string indexstr;
	std::string identityidstr;

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,identityid);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);

		now.SetToGMTime();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblIntroductionPuzzleRequests WHERE Day=? AND IdentityID=?;");
		st2.Bind(0,now.Format("%Y-%m-%d"));
		st2.Bind(1,identityid);
		st2.Step();

		index=0;
		if(st2.RowReturned())
		{
			if(st2.ResultNull(0)==false)
			{
				st2.ResultInt(0,index);
				index++;
			}
		}
		st2.Finalize();

		StringFunctions::Convert(index,indexstr);
		StringFunctions::Convert(identityid,identityidstr);

		message.SetName("ClientGet");
		message["URI"]=publickey+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|IntroductionPuzzle|"+indexstr+".xml";
		message["Identifier"]="IntroductionPuzzleRequester|"+identityidstr+"|"+indexstr+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";		// 1 MB

		m_fcp->SendMessage(message);
		
		m_requesting.push_back(identityid);
	}

	m_ids[identityid]=true;
}
