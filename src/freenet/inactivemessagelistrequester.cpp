#include "../../include/freenet/inactivemessagelistrequester.h"

#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

InactiveMessageListRequester::InactiveMessageListRequester(SQLite3DB::DB *db):MessageListRequester(db)
{
	Initialize();
}

InactiveMessageListRequester::InactiveMessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):MessageListRequester(db,fcp)
{
	Initialize();
}

void InactiveMessageListRequester::Initialize()
{
	m_fcpuniquename="InactiveMessageListRequester";
	std::string tempval="";

	m_maxrequests=0;
	Option option(m_db);
	option.GetInt("MaxMessageListRequests",m_maxrequests);

	// inactive identities get 1/2 of the max requests option -  active identities get 1/2 + any remaining if not evenly divisible
	m_maxrequests=(m_maxrequests/2);

	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxMessageListRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxMessageListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	tempval="";
	option.Get("LocalTrustOverridesPeerTrust",tempval);
	if(tempval=="true")
	{
		m_localtrustoverrides=true;
	}
	else
	{
		m_localtrustoverrides=false;
	}

	tempval="";
	option.Get("SaveMessagesFromNewBoards",tempval);
	if(tempval=="true")
	{
		m_savetonewboards=true;
	}
	else
	{
		m_savetonewboards=false;
	}

	m_messagedownloadmaxdaysbackward=5;
	tempval="5";
	option.Get("MessageDownloadMaxDaysBackward",tempval);
	StringFunctions::Convert(tempval,m_messagedownloadmaxdaysbackward);

}

void InactiveMessageListRequester::PopulateIDList()
{
	Poco::DateTime date;
	Poco::DateTime yesterday=date-Poco::Timespan(1,0,0,0,0);
	int id;
	SQLite3DB::Transaction trans(m_db);

	m_ids.clear();

	SQLite3DB::Statement st;

	// only selects, deferred OK
	trans.Begin();

	// select identities we want to query (we've seen them today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	if(m_localtrustoverrides==false)
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>=? AND (LastMessageDate IS NULL OR LastMessageDate<?) AND (LocalMessageTrust IS NULL OR LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	else
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>=? AND (LastMessageDate IS NULL OR LastMessageDate<?) AND (LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (LocalMessageTrust IS NULL AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	st.Bind(0, Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Bind(1, Poco::DateTimeFormatter::format(yesterday,"%Y-%m-%d"));

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
