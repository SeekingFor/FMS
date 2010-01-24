#include "../include/localidentity.h"

#ifdef XMEM
	#include <xmem.h>
#endif

LocalIdentity::LocalIdentity(SQLite3DB::DB *db):IDatabase(db)
{
	Initialize();
}

void LocalIdentity::Initialize()
{
	m_id=-1;
	m_name="";
	m_publickey="";
	m_privatekey="";
	m_active=false;
}

const bool LocalIdentity::Load(const int id)
{

	Initialize();

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID,Name,PublicKey,PrivateKey,Active FROM tblLocalIdentity WHERE LocalIdentityID=?;");
	st.Bind(0,id);
	st.Step();
	if(st.RowReturned())
	{
		std::string active("");

		st.ResultInt(0,m_id);
		st.ResultText(1,m_name);
		st.ResultText(2,m_publickey);
		st.ResultText(3,m_privatekey);
		st.ResultText(4,active);
		if(active=="true")
		{
			m_active=true;
		}
		else
		{
			m_active=false;
		}
		return true;
	}
	else
	{
		return false;
	}

}

const bool LocalIdentity::Load(const std::string &name)
{
	Initialize();

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE Name=?;");
	st.Bind(0,name);
	st.Step();
	if(st.RowReturned())
	{
		int id=-1;
		st.ResultInt(0,id);
		return Load(id);
	}
	else
	{
		return false;
	}
}
