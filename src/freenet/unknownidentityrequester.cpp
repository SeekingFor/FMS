#include "../../include/freenet/unknownidentityrequester.h"
#include "../../include/option.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

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
	int previnsertcount=0;

	// get count of identities added more than 24 hours ago - if 0 then we will accept more than 100 identities now
	Poco::DateTime onedayago;
	onedayago-=Poco::Timespan(1,0,0,0,0);
	SQLite3DB::Statement st=m_db->Prepare("SELECT COUNT(*) FROM tblIdentity WHERE DateAdded<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(onedayago,"%Y-%m-%d %H:%M:%S"));
	st.Step();
	if(st.RowReturned())
	{
		if(st.ResultNull(0)==false)
		{
			st.ResultInt(0,previnsertcount);
		}
	}
	else
	{
		m_log->error("UnknownIdentityRequester::Initialize couldn't get count of identities added more than 24 hours ago");
	}

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
	if(previnsertcount==0)
	{
		m_maxrequests=(std::max)(m_maxrequests,10);
	}
	else
	{
		std::string previnsertcountstr;
		StringFunctions::Convert(previnsertcount,previnsertcountstr);
		m_log->trace("UnknownIdentityRequester::Initialize previnsertcount is "+previnsertcountstr);
	}
}

void UnknownIdentityRequester::PopulateIDList()
{
	int id;
	int count=0;
	std::string countstr("0");

	m_ids.clear();

	m_db->Execute("BEGIN;");

	// select identities we want to query (haven't seen at all) - sort by their trust level (descending)
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen IS NULL ORDER BY RANDOM();");
	st.Step();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[std::pair<long,long>(count,id)].m_requested=false;
		st.Step();
		count+=1;
	}

	m_db->Execute("COMMIT;");

	StringFunctions::Convert(count,countstr);
	m_log->trace("UnknownIdentityRequester::PopulateIDList populated "+countstr+" ids");
}
