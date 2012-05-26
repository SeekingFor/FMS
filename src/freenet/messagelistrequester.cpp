#include "../../include/freenet/messagelistrequester.h"
#include "../../include/freenet/identitypublickeycache.h"
#include "../../include/stringfunctions.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Timestamp.h>

#ifdef XMEM
	#include <xmem.h>
#endif

MessageListRequester::MessageListRequester(SQLite3DB::DB *db):IMessageListRequester<long>(db)
{
	Initialize();
}

MessageListRequester::MessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IMessageListRequester<long>(db,fcp)
{
	Initialize();
}

const long MessageListRequester::GetIDFromIdentifier(const std::string &identifier)
{
	long id;
	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	StringFunctions::Convert(idparts[1],id);
	return id;
}

void MessageListRequester::Initialize()
{
	m_fcpuniquename="ActiveMessageListRequester";
	std::string tempval("");
	m_maxrequests=0;
	Option option(m_db);

	option.GetInt("MaxMessageListRequests",m_maxrequests);

	// active identities get 1/2 of the max requests option + any remaining if not evenly divisible - inactive identities get 1/2
	m_maxrequests=(m_maxrequests/2)+(m_maxrequests%2);

	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxMessageListRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxMessageListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
}

void MessageListRequester::PopulateIDList()
{
	Poco::DateTime date;
	Poco::DateTime yesterday=date-Poco::Timespan(1,0,0,0,0);
	int id;
	SQLite3DB::Transaction trans(m_db);

	m_ids.clear();

	SQLite3DB::Statement st;

	// select identities we want to query (we've seen them today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	if(m_localtrustoverrides==false)
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>=? AND (LastMessageDate>=?) AND (LocalMessageTrust IS NULL OR LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	else
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>=? AND (LastMessageDate>=?) AND (LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (LocalMessageTrust IS NULL AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	st.Bind(0, Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Bind(1, Poco::DateTimeFormatter::format(yesterday,"%Y-%m-%d"));

	// only selects, deferred OK
	trans.Begin();

	trans.Step(st);

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id].m_requested=false;
		trans.Step(st);
	}

	trans.Finalize(st);
	trans.Commit();
}

void MessageListRequester::StartRequest(const long &id)
{
	Poco::DateTime now;
	FCPv2::Message message;
	std::string publickey;
	int index=0;
	std::string indexstr;
	std::string identityidstr;
	IdentityPublicKeyCache pkcache(m_db);

	if(pkcache.PublicKey(id,publickey))
	{

		now=Poco::Timestamp();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblMessageListRequests WHERE Day=? AND IdentityID=?;");
		st2.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
		st2.Bind(1,id);
		st2.Step();

		index=0;
		if(st2.RowReturned())
		{
			if(st2.ResultNull(0)==false)
			{
				st2.ResultInt(0,index);
				// don't increment index here - the node will let us know if there is a new edition
				// 2008-05-31 - well actually the node isn't reliably retreiving the latest edition for USKs, so we DO need to increment the index
				index++;
			}
		}
		st2.Finalize();

		StringFunctions::Convert(index,indexstr);
		StringFunctions::Convert(id,identityidstr);

		message.SetName("ClientGet");
		message["URI"]="USK"+publickey.substr(3)+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y.%m.%d")+"|MessageList/"+indexstr+"/MessageList.xml";
		message["IgnoreUSKDatehints"]="true"; // per-day key, DATEHINTs useless
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|_|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|"+message["URI"];
		message["PriorityClass"]=m_defaultrequestpriorityclassstr;
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";

		m_fcp->Send(message);

		StartedRequest(id,message["Identifier"]);
	}

	m_ids[id].m_requested=true;
}
