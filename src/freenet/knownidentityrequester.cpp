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

	// known identities get 4/5 + any remaining if not evenly divisible - unknown identities get 1/5 of the max requests option
	m_maxrequests=((m_maxrequests*4)/5)+(m_maxrequests%5);

	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxIdentityRequests is currently set at less than 1.  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxIdentityRequests is currently set at more than 100.  This value might be incorrectly configured.");
	}
}

void KnownIdentityRequester::PopulateIDList()
{
	Poco::DateTime date;
	int id;
	int count=0;

	date.assign(date.year(),date.month(),date.day(),0,0,0);

	// select identities we want to query (haven't seen yet today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen IS NOT NULL AND LastSeen<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"' AND tblIdentity.FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[std::pair<long,long>(count,id)]=false;
		st.Step();
		count+=1;
	}
}
