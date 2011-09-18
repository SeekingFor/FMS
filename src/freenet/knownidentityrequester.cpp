#include "../../include/freenet/knownidentityrequester.h"
#include "../../include/option.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

KnownIdentityRequester::KnownIdentityRequester(SQLite3DB::DB *db):IdentityRequester(db)
{
	Initialize();
}

KnownIdentityRequester::KnownIdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IdentityRequester(db,fcp)
{
	Initialize();
}

void KnownIdentityRequester::Initialize()
{
	m_fcpuniquename="KnownIdentityRequester";
	Option option(m_db);
	option.GetInt("MaxIdentityRequests",m_maxrequests);

	// known identities get 2/5 + any remaining if not evenly divisible, inactive identities get 2/5 and unknown identities get 1/5
	m_maxrequests=((m_maxrequests*2)/5)+(m_maxrequests%5);

	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("KnownIdentityRequester::Initialize Option MaxIdentityRequests is currently set at less than 1.  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("KnownIdentityRequester::Initialize Option MaxIdentityRequests is currently set at more than 100.  This value might be incorrectly configured.");
	}
}

void KnownIdentityRequester::PopulateIDList()
{
	Poco::DateTime date;
	Poco::DateTime weekago;
	int id;
	int count=0;
	SQLite3DB::Transaction trans(m_db);

	date.assign(date.year(),date.month(),date.day(),0,0,0);
	weekago-=Poco::Timespan(7,0,0,0,0);
	weekago.assign(weekago.year(),weekago.month(),weekago.day(),0,0,0);

	// only selects, deferred OK
	trans.Begin();

	// select identities we want to query (haven't seen yet today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen IS NOT NULL AND LastSeen<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"' AND LastSeen>='"+Poco::DateTimeFormatter::format(weekago,"%Y-%m-%d %H:%M:%S")+"' AND tblIdentity.FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LastSeen DESC, LocalMessageTrust+LocalTrustListTrust DESC;");
	trans.Step(st);

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[std::pair<long,long>(count,id)].m_requested=false;
		trans.Step(st);
		count+=1;
	}

	trans.Finalize(st);
	trans.Commit();
}
