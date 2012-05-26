#include "../../include/freenet/oldmessagelistrequester.h"
#include "../../include/freenet/identitypublickeycache.h"
#include "../../include/stringfunctions.h"

OldMessageListRequester::OldMessageListRequester(SQLite3DB::DB *db):IMessageListRequester<std::string>(db)
{
	Initialize();
}

OldMessageListRequester::OldMessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IMessageListRequester<std::string>(db,fcp)
{
	Initialize();
}

const std::string OldMessageListRequester::GetIDFromIdentifier(const std::string &identifier)
{
	std::string id("");
	std::vector<std::string> idparts;

	StringFunctions::Split(identifier,"|",idparts);

	if(idparts.size()>4)
	{
		id=idparts[4]+"|"+idparts[1]+"|"+idparts[2];
		return id;
	}
	else
	{
		return "";
	}

}

void OldMessageListRequester::Initialize()
{
	std::string tempval("");
	Option option(m_db);

	m_fcpuniquename="OldMessageListRequester";
	option.GetInt("MaxOldMessageListRequests",m_maxrequests);

	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxOldMessageListRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxOldMessageListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	m_reverserequest=true;

}

void OldMessageListRequester::PopulateIDList()
{
	Poco::DateTime date;

	m_ids.clear();

	SQLite3DB::Statement st;
	SQLite3DB::Transaction trans(m_db);
	
	if(m_localtrustoverrides==false)
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID, COALESCE(i.RI+1,1) FROM tblIdentity LEFT JOIN \
						(SELECT IdentityID, MAX(RequestIndex) AS RI FROM tblMessageListRequests WHERE Day=? GROUP BY IdentityID) AS i ON tblIdentity.IdentityID=i.IdentityID \
						WHERE tblIdentity.PublicKey IS NOT NULL AND tblIdentity.PublicKey <> '' AND tblIdentity.LastSeen IS NOT NULL \
						AND (LocalMessageTrust IS NULL OR LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) \
						AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount');");
	}
	else
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID, COALESCE(i.RI+1,1) FROM tblIdentity LEFT JOIN \
						(SELECT IdentityID, MAX(RequestIndex) AS RI FROM tblMessageListRequests WHERE Day=? GROUP BY IdentityID) AS i ON tblIdentity.IdentityID=i.IdentityID \
						WHERE tblIdentity.PublicKey IS NOT NULL AND tblIdentity.PublicKey <> '' AND tblIdentity.LastSeen IS NOT NULL \
						AND (LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (LocalMessageTrust IS NULL AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) \
						AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount');");
	}

	// only selects, deferred OK
	trans.Begin();

	// only load up 1 day back, and we'll dynamically load previous days as they are requested
	long i=1;
	//for(long i=1; i<=m_messagedownloadmaxdaysbackward; i++)
	{
		date=Poco::DateTime();
		date-=Poco::Timespan(i,0,0,0,0);

		st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
		trans.Step(st);

		while(st.RowReturned())
		{
			std::string identityid("");
			std::string index("");

			st.ResultText(0,identityid);
			st.ResultText(1,index);

			m_ids[Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"|"+identityid+"|"+index].m_requested=false;

			trans.Step(st);
		}

		trans.Reset(st);

	}

	trans.Finalize(st);
	trans.Commit();

}

void OldMessageListRequester::PostHandleAllData(FCPv2::Message &message)
{
	m_ids.erase(GetIDFromIdentifier(message["Identifier"]));
}

void OldMessageListRequester::PostHandleGetFailed(FCPv2::Message &message)
{
	m_ids.erase(GetIDFromIdentifier(message["Identifier"]));
}

void OldMessageListRequester::StartRequest(const std::string &id)
{
	FCPv2::Message message;
	std::vector<std::string> idparts;
	std::string publickey("");
	std::string day("");
	std::string identityidstr("");
	std::string indexstr("");
	IdentityPublicKeyCache pkcache(m_db);
	long identityid=0;
	Poco::DateTime now;
	Poco::DateTime date;
	int tz=0;

	StringFunctions::Split(id,"|",idparts);
	if(idparts.size()==3)
	{
		StringFunctions::Convert(idparts[1],identityid);
	}

	if(pkcache.PublicKey(identityid,publickey) && idparts.size()==3)
	{

		day=idparts[0];
		identityidstr=idparts[1];
		indexstr=idparts[2];

		Poco::DateTimeParser::tryParse(day,date,tz);

		message.SetName("ClientGet");
		message["URI"]="USK"+publickey.substr(3)+m_messagebase+"|"+StringFunctions::Replace(day,"-",".")+"|MessageList/"+indexstr+"/MessageList.xml";
		message["IgnoreUSKDatehints"]="true"; // per-day key, DATEHINTs useless
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|_|"+day+"|"+message["URI"];
		message["PriorityClass"]=m_defaultrequestpriorityclassstr;
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";

		m_fcp->Send(message);

		StartedRequest(id,message["Identifier"]);

		m_log->trace("OldMessageListRequester::StartRequest started request for "+message["Identifier"]);
	}

	m_ids[id].m_requested=true;

	date-=Poco::Timespan(1,0,0,0,0);
	if(Poco::Timespan(now-date).days()<=m_messagedownloadmaxdaysbackward)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT COALESCE(MAX(RequestIndex)+1,1) FROM tblMessageListRequests WHERE IdentityID=? AND Day=?;");
		st.Bind(0,identityidstr);
		st.Bind(1,day);
		st.Step();
		if(st.RowReturned())
		{
			int index=-1;
			st.ResultInt(0,index);
			StringFunctions::Convert(index,indexstr);
			m_ids[Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"|"+identityidstr+"|"+indexstr];
		}
	}

}
