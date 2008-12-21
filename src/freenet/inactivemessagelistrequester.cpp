#include "../../include/freenet/inactivemessagelistrequester.h"

#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

InactiveMessageListRequester::InactiveMessageListRequester()
{
	Initialize();
}

InactiveMessageListRequester::InactiveMessageListRequester(FCPv2 *fcp):MessageListRequester(fcp)
{
	Initialize();
}

void InactiveMessageListRequester::Initialize()
{
	m_fcpuniquename="InactiveMessageListRequester";
	std::string tempval="";

	m_maxrequests=0;
	Option::Instance()->GetInt("MaxMessageListRequests",m_maxrequests);

	// inactive identities get 1/2 of the max requests option -  active identities get 1/2 + any remaining if not evenly divisible
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

	tempval="";
	Option::Instance()->Get("LocalTrustOverridesPeerTrust",tempval);
	if(tempval=="true")
	{
		m_localtrustoverrides=true;
	}
	else
	{
		m_localtrustoverrides=false;
	}

	tempval="";
	Option::Instance()->Get("SaveMessagesFromNewBoards",tempval);
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
	Option::Instance()->Get("MessageDownloadMaxDaysBackward",tempval);
	StringFunctions::Convert(tempval,m_messagedownloadmaxdaysbackward);

}

void InactiveMessageListRequester::PopulateIDList()
{
	Poco::DateTime date;
	Poco::DateTime yesterday=date-Poco::Timespan(1,0,0,0,0);
	int id;

	SQLite3DB::Statement st;

	// select identities we want to query (we've seen them today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	if(m_localtrustoverrides==false)
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity INNER JOIN vwIdentityStats ON tblIdentity.IdentityID=vwIdentityStats.IdentityID WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND (vwIdentityStats.LastMessageDate IS NULL OR vwIdentityStats.LastMessageDate<'"+Poco::DateTimeFormatter::format(yesterday,"%Y-%m-%d")+"') AND (LocalMessageTrust IS NULL OR LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	else
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity INNER JOIN vwIdentityStats ON tblIdentity.IdentityID=vwIdentityStats.IdentityID WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND (vwIdentityStats.LastMessageDate IS NULL OR vwIdentityStats.LastMessageDate<'"+Poco::DateTimeFormatter::format(yesterday,"%Y-%m-%d")+"') AND (LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (LocalMessageTrust IS NULL AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}
