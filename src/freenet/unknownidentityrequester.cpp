#include "../../include/freenet/unknownidentityrequester.h"
#include "../../include/option.h"

#ifdef XMEM
	#include <xmem.h>
#endif

UnknownIdentityRequester::UnknownIdentityRequester(SQLite3DB::DB *db):IdentityRequester(db)
{
	Initialize();
}

UnknownIdentityRequester::UnknownIdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IdentityRequester(db,fcp)
{
	Initialize();
}

void UnknownIdentityRequester::Initialize()
{
	Option option(m_db);

	m_fcpuniquename="UnknownIdentityRequester";
	option.GetInt("MaxIdentityRequests",m_maxrequests);

	// unknown identities get 1/5 of the max requests option - known identities get 4/5 + any remaining if not evenly divisible
	m_maxrequests=(m_maxrequests/5);

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

void UnknownIdentityRequester::PopulateIDList()
{
	int id;

	// select identities we want to query (haven't seen at all) - sort by their trust level (descending)
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen IS NULL ORDER BY RANDOM();");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}
