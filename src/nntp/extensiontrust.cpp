#include "../../include/nntp/extensiontrust.h"
#include "../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

TrustExtension::TrustExtension(SQLite3DB::DB *db):IDatabase(db)
{
	m_localidentityid=-1;
}

TrustExtension::TrustExtension(SQLite3DB::DB *db, const int &localidentityid):IDatabase(db)
{
	m_localidentityid=localidentityid;
}

const int TrustExtension::GetIdentityID(const std::string &nntpname)
{
	std::vector<std::string> nameparts;
	StringFunctions::Split(nntpname,"@",nameparts);

	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID, PublicKey FROM tblIdentity WHERE Name=? AND PublicKey IS NOT NULL AND PublicKey  <> '' ;");
	st.Bind(0,nameparts[0]);
	st.Step();

	while(st.RowReturned())
	{
		int id=-1;
		std::vector<std::string> keyparts;
		std::string publickey="";

		st.ResultText(1,publickey);
		StringFunctions::SplitMultiple(publickey,"@,",keyparts);

		if(keyparts.size()>1)
		{
			if(nameparts[0]+"@"+keyparts[1]==nntpname)
			{
				st.ResultInt(0,id);
				return id;
			}
		}

		st.Step();
	}

	return -1;

}

const bool TrustExtension::GetMessageTrust(const std::string &nntpname, int &trust)
{
	if(m_localidentityid>=0)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("SELECT LocalMessageTrust FROM tblIdentityTrust WHERE LocalIdentityID=? AND IdentityID=?;");
			st.Bind(0,m_localidentityid);
			st.Bind(1,id);
			st.Step();

			if(st.RowReturned())
			{
				int tr=-1;
				if(st.ResultNull(0)==false)
				{
					st.ResultInt(0,tr);
				}
				trust=tr;
			}
			else
			{
				trust=-1;
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::GetPeerMessageTrust(const std::string &nntpname, int &trust)
{
	if(m_localidentityid>=0)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("SELECT PeerMessageTrust FROM tblIdentity WHERE IdentityID=?;");
			st.Bind(0,id);
			st.Step();

			if(st.RowReturned())
			{
				int tr=-1;
				if(st.ResultNull(0)==false)
				{
					st.ResultInt(0,tr);
				}
				trust=tr;
			}
			else
			{
				trust=-1;
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::GetPeerTrustListTrust(const std::string &nntpname, int &trust)
{
	if(m_localidentityid>=0)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("SELECT PeerTrustListTrust FROM tblIdentity WHERE IdentityID=?;");
			st.Bind(0,id);
			st.Step();

			if(st.RowReturned())
			{
				int tr=-1;
				if(st.ResultNull(0)==false)
				{
					st.ResultInt(0,tr);
				}
				trust=tr;
			}
			else
			{
				trust=-1;
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::GetTrustList(std::map<std::string,trust> &trustlist)
{
	if(m_localidentityid>=0)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT tblIdentityTrust.LocalMessageTrust,tblIdentityTrust.LocalTrustListTrust,tblIdentity.Name,tblIdentity.PublicKey,tblIdentityTrust.MessageTrustComment,tblIdentityTrust.TrustListTrustComment,tblIdentity.PeerMessageTrust,tblIdentity.PeerTrustListTrust FROM tblIdentityTrust INNER JOIN tblIdentity ON tblIdentityTrust.IdentityID=tblIdentity.IdentityID WHERE tblIdentityTrust.LocalIdentityID=? AND tblIdentity.Name IS NOT NULL AND tblIdentity.PublicKey IS NOT NULL AND tblIdentity.PublicKey <> '' ;");
		st.Bind(0,m_localidentityid);
		st.Step();
		while(st.RowReturned())
		{
			int messagetrust=-1;
			int trustlisttrust=-1;
			int peermessagetrust=-1;
			int peertrustlisttrust=-1;
			std::string messagetrustcomment="";
			std::string trustlisttrustcomment="";
			std::string name="";
			std::string publickey="";
			std::vector<std::string> keyparts;
			std::string nntpname="";

			if(st.ResultNull(0)==false)
			{
				st.ResultInt(0,messagetrust);
			}
			if(st.ResultNull(1)==false)
			{
				st.ResultInt(1,trustlisttrust);
			}
			st.ResultText(2,name);
			st.ResultText(3,publickey);
			st.ResultText(4,messagetrustcomment);
			st.ResultText(5,trustlisttrustcomment);
			if(st.ResultNull(6)==false)
			{
				st.ResultInt(6,peermessagetrust);
			}
			if(st.ResultNull(7)==false)
			{
				st.ResultInt(7,peertrustlisttrust);
			}

			StringFunctions::SplitMultiple(publickey,"@,",keyparts);
			if(keyparts.size()>1)
			{
				nntpname=name+"@"+keyparts[1];
			}

			trustlist[nntpname]=trust(messagetrust,peermessagetrust,messagetrustcomment,trustlisttrust,peertrustlisttrust,trustlisttrustcomment);

			st.Step();
		}
		return true;
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::GetTrustListTrust(const std::string &nntpname, int &trust)
{
	if(m_localidentityid>=0)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("SELECT LocalTrustListTrust FROM tblIdentityTrust WHERE LocalIdentityID=? AND IdentityID=?;");
			st.Bind(0,m_localidentityid);
			st.Bind(1,id);
			st.Step();

			if(st.RowReturned())
			{
				int tr=-1;
				if(st.ResultNull(0)==false)
				{
					st.ResultInt(0,tr);
				}
				trust=tr;
			}
			else
			{
				trust=-1;
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::SetMessageTrust(const std::string &nntpname, const int trust)
{
	if(m_localidentityid>=0 && trust>=-1 && trust<=100)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentityTrust SET LocalMessageTrust=? WHERE LocalIdentityID=? AND IdentityID=?;");
			if(trust==-1)
			{
				st.Bind(0);
			}
			else
			{
				st.Bind(0,trust);
			}
			st.Bind(1,m_localidentityid);
			st.Bind(2,id);
			st.Step();

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::SetMessageTrustComment(const std::string &nntpname, const std::string &comment)
{
	if(m_localidentityid>=0)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentityTrust SET MessageTrustComment=? WHERE LocalIdentityID=? AND IdentityID=?;");
			if(comment=="")
			{
				st.Bind(0);
			}
			else
			{
				st.Bind(0,comment);
			}
			st.Bind(1,m_localidentityid);
			st.Bind(2,id);
			st.Step();

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::SetTrustListTrust(const std::string &nntpname, const int trust)
{
	if(m_localidentityid>=0 && trust>=-1 && trust<=100)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentityTrust SET LocalTrustListTrust=? WHERE LocalIdentityID=? AND IdentityID=?;");
			if(trust==-1)
			{
				st.Bind(0);
			}
			else
			{
				st.Bind(0,trust);
			}
			st.Bind(1,m_localidentityid);
			st.Bind(2,id);
			st.Step();

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool TrustExtension::SetTrustListTrustComment(const std::string &nntpname, const std::string &comment)
{
	if(m_localidentityid>=0)
	{
		int id=GetIdentityID(nntpname);
		if(id>=0)
		{
			SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentityTrust SET TrustListTrustComment=? WHERE LocalIdentityID=? AND IdentityID=?;");
			if(comment=="")
			{
				st.Bind(0);
			}
			else
			{
				st.Bind(0,comment);
			}
			st.Bind(1,m_localidentityid);
			st.Bind(2,id);
			st.Step();

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}
