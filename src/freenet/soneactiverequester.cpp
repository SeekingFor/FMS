#include "../../include/freenet/soneactiverequester.h"

SoneActiveRequester::SoneActiveRequester(SQLite3DB::DB *db):SoneRequester(db)
{
	Initialize();
}

SoneActiveRequester::SoneActiveRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):SoneRequester(db,fcp)
{
	Initialize();
}

void SoneActiveRequester::Initialize()
{
	Option option(m_db);

	m_fcpuniquename="SoneActiveRequester";
	option.GetInt("SoneMaxRequests",m_maxrequests);
	option.Get("SoneBoardName",m_soneboardname);
	option.GetBool("LocalTrustOverridesPeerTrust",m_localtrustoverrides);
	option.GetInt("DeleteMessagesOlderThan",m_deletemessagesolderthan);

	// recent sones get 1/2, inactive sones get 1/2 + any remaining if not evenly divisible
	m_maxrequests=(m_maxrequests/2);

	if(m_maxrequests>100)
	{
		m_log->warning("Option SoneActiveRequester is currently set at more than 100.  This value might be incorrectly configured.");
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID FROM tblBoard WHERE BoardName=?;");
	st.Bind(0,m_soneboardname);
	st.Step();
	if(st.RowReturned()==true)
	{
		st.ResultInt(0,m_soneboardid);
	}
	else
	{
		SQLite3DB::Statement inst=m_db->Prepare("INSERT INTO tblBoard(BoardName,DateAdded) VALUES(?,datetime('now'));");
		inst.Bind(0,m_soneboardname);
		inst.Step(true);
		m_soneboardid=inst.GetLastInsertRowID();
	}

}

void SoneActiveRequester::PopulateIDList()
{
	SQLite3DB::Transaction trans(m_db);

	// only selects, deferred OK
	trans.Begin();

	std::string sql;

	sql="SELECT tblIdentity.IdentityID ";
	sql+="FROM tblIdentity INNER JOIN tblWOTIdentityContext ON tblIdentity.IdentityID=tblWOTIdentityContext.IdentityID ";
	sql+="WHERE Context='Sone' ";
	if(m_localtrustoverrides==false)
	{
		sql+="AND (tblIdentity.LocalMessageTrust IS NULL OR tblIdentity.LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) ";
		sql+="AND (tblIdentity.PeerMessageTrust IS NULL OR tblIdentity.PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) ";
	}
	else
	{
		sql+="AND (tblIdentity.LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (tblIdentity.LocalMessageTrust IS NULL AND (tblIdentity.PeerMessageTrust IS NULL OR tblIdentity.PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) ";
	}
	sql+="AND tblIdentity.Name <> '' AND tblIdentity.FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ";
	sql+="AND (tblIdentity.SoneLastSeen>datetime('now','-1 hours')) ";
	sql+="ORDER BY tblIdentity.SoneLastRequest DESC ";
	sql+=";";

	int count=0;
	std::string countstr("");
	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		int id=0;
		st.ResultInt(0,id);
		m_ids[std::pair<long,long>(count,id)].m_requested=false;
		st.Step();
		count++;
	}

	trans.Finalize(st);
	trans.Commit();

	StringFunctions::Convert(count,countstr);
	m_log->trace(m_fcpuniquename+"::PopulateIDList populated "+countstr+" ids");

}
