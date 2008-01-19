#include "../../include/freenet/trustlistxml.h"
#include "../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

TrustListXML::TrustListXML()
{
	Initialize();
}

void TrustListXML::AddTrust(const std::string &identity, const long messagetrust, const long trustlisttrust)
{
	m_trust.push_back(trust(identity,messagetrust,trustlisttrust));
}

std::string TrustListXML::GetIdentity(const long index)
{
	if(index>=0 && index<m_trust.size())
	{
		return m_trust[index].m_identity;
	}
	else
	{
		return "";
	}
}

long TrustListXML::GetMessageTrust(const long index)
{
	if(index>=0 && index<m_trust.size())
	{
		return m_trust[index].m_messagetrust;
	}
	else
	{
		return -1;
	}	
}

long TrustListXML::GetTrustListTrust(const long index)
{
	if(index>=0 && index<m_trust.size())
	{
		return m_trust[index].m_trustlisttrust;
	}
	else
	{
		return -1;
	}
}

std::string TrustListXML::GetXML()
{
	TiXmlDocument td;
	TiXmlDeclaration *tdec=new TiXmlDeclaration("1.0","UTF-8","");
	TiXmlElement *tid;
	TiXmlPrinter tp;

	td.LinkEndChild(tdec);
	tid=new TiXmlElement("TrustList");
	td.LinkEndChild(tid);

	for(std::vector<trust>::iterator i=m_trust.begin(); i!=m_trust.end(); i++)
	{
		std::string messagetrust;
		std::string trustlisttrust;
		StringFunctions::Convert((*i).m_messagetrust,messagetrust);
		StringFunctions::Convert((*i).m_trustlisttrust,trustlisttrust);
		TiXmlElement *tr=new TiXmlElement("Trust");
		tid->LinkEndChild(tr);
		tr->LinkEndChild(XMLCreateCDATAElement("Identity",(*i).m_identity));
		tr->LinkEndChild(XMLCreateTextElement("MessageTrustLevel",messagetrust));
		tr->LinkEndChild(XMLCreateTextElement("TrustListTrustLevel",trustlisttrust));
	}

	td.Accept(&tp);
	return std::string(tp.CStr());
}

void TrustListXML::Initialize()
{
	m_trust.clear();
}

const bool TrustListXML::ParseXML(const std::string &xml)
{
	TiXmlDocument td;
	td.Parse(xml.c_str());

	if(!td.Error())
	{
		std::string identity;
		std::string messagetruststr;
		std::string trustlisttruststr;
		long messagetrust;
		long trustlisttrust;
		TiXmlText *txt;
		TiXmlHandle hnd(&td);
		TiXmlNode *node;

		Initialize();

		node=hnd.FirstChild("TrustList").FirstChild("Trust").ToElement();
		while(node)
		{
			identity="";
			messagetrust=-1;
			trustlisttrust=-1;

			TiXmlHandle hnd2(node);
			txt=hnd2.FirstChild("Identity").FirstChild().ToText();
			if(txt)
			{
				identity=txt->ValueStr();
			}
			txt=hnd2.FirstChild("MessageTrustLevel").FirstChild().ToText();
			if(txt)
			{
				messagetruststr=txt->ValueStr();
				StringFunctions::Convert(messagetruststr,messagetrust);
			}
			txt=hnd2.FirstChild("TrustListTrustLevel").FirstChild().ToText();
			if(txt)
			{
				trustlisttruststr=txt->ValueStr();
				StringFunctions::Convert(trustlisttruststr,trustlisttrust);
			}

			if(identity!="" && messagetrust>=0 && messagetrust<=100 && trustlisttrust>=0 && trustlisttrust<=100)
			{
				m_trust.push_back(trust(identity,messagetrust,trustlisttrust));
			}
			else
			{
				m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"TrustListXML::ParseXML malformed Trust in TrustList.xml");
			}
			
			node=node->NextSibling("Trust");
		}
		return true;

	}
	else
	{
		return false;
	}
}
