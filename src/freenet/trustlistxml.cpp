#include "../../include/freenet/trustlistxml.h"
#include "../../include/stringfunctions.h"

#include <algorithm>

#ifdef XMEM
	#include <xmem.h>
#endif

TrustListXML::TrustListXML()
{
	Initialize();
}

void TrustListXML::AddTrust(const std::string &identity, const long messagetrust, const long trustlisttrust, const std::string &messagetrustcomment, const std::string &trustlisttrustcomment, bool isfms, bool iswot)
{
	m_trust.push_back(trust(identity,messagetrust,trustlisttrust,messagetrustcomment,trustlisttrustcomment,isfms,iswot));
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

std::string TrustListXML::GetMessageTrustComment(const long index)
{
	if(index>=0 && index<m_trust.size())
	{
		return m_trust[index].m_messagetrustcomment;
	}
	else
	{
		return "";
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

std::string TrustListXML::GetTrustListTrustComment(const long index)
{
	if(index>=0 && index<m_trust.size())
	{
		return m_trust[index].m_trustlisttrustcomment;
	}
	else
	{
		return "";
	}
}

bool TrustListXML::GetIsFMS(const long index)
{
	if(index>=0 && index<m_trust.size())
	{
		return m_trust[index].m_isfms;
	}
	else
	{
		return false;
	}
}

bool TrustListXML::GetIsWOT(const long index)
{
	if(index>=0 && index<m_trust.size())
	{
		return m_trust[index].m_iswot;
	}
	else
	{
		return false;
	}
}

std::string TrustListXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("TrustList");

	doc->appendChild(root);

	if(root)
	{
		root->setAttribute("Version","1.0");
	}

	for(std::vector<trust>::iterator i=m_trust.begin(); i!=m_trust.end(); i++)
	{
		std::string messagetrust;
		std::string trustlisttrust;
		StringFunctions::Convert((*i).m_messagetrust,messagetrust);
		StringFunctions::Convert((*i).m_trustlisttrust,trustlisttrust);
		Poco::AutoPtr<Poco::XML::Element> tr=doc->createElement("Trust");
		root->appendChild(tr);
		tr->appendChild(XMLCreateCDATAElement(doc,"Identity",(*i).m_identity));
		if((*i).m_messagetrust>=0)
		{
			tr->appendChild(XMLCreateTextElement(doc,"MessageTrustLevel",messagetrust));
		}
		if((*i).m_trustlisttrust>=0)
		{
			tr->appendChild(XMLCreateTextElement(doc,"TrustListTrustLevel",trustlisttrust));
		}
		if((*i).m_messagetrustcomment!="")
		{
			tr->appendChild(XMLCreateTextElement(doc,"MessageTrustComment",(*i).m_messagetrustcomment));
		}
		if((*i).m_trustlisttrustcomment!="")
		{
			tr->appendChild(XMLCreateTextElement(doc,"TrustListTrustComment",(*i).m_trustlisttrustcomment));
		}
		tr->appendChild(XMLCreateBooleanElement(doc,"IsFMS",(*i).m_isfms));
		tr->appendChild(XMLCreateBooleanElement(doc,"IsWOT",(*i).m_iswot));
	}

	return GenerateXML(doc);
}

void TrustListXML::Initialize()
{
	m_trust.clear();
}

const bool TrustListXML::ParseXML(const std::string &xml)
{

	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"TrustList");
		Poco::XML::Element *trustel=NULL;
		Poco::XML::Element *txt=NULL;
		std::string tlver="";

		if(root)
		{
			tlver=root->getAttribute("Version");
		}

		std::vector<std::string> foundkeys;

		trustel=XMLGetFirstChild(root,"Trust");
		while(trustel)
		{
			std::string identity="";
			int messagetrust=-1;
			int trustlisttrust=-1;
			std::string messagetrustcomment="";
			std::string trustlisttrustcomment="";
			bool isfms=false;
			bool iswot=false;

			txt=XMLGetFirstChild(trustel,"Identity");
			if(txt)
			{
				if(txt->firstChild())
				{
					identity=SanitizeSingleString(txt->firstChild()->getNodeValue());
				}
			}
			txt=XMLGetFirstChild(trustel,"MessageTrustLevel");
			if(txt)
			{
				if(txt->firstChild())
				{
					std::string mtl=txt->firstChild()->getNodeValue();
					StringFunctions::Convert(mtl,messagetrust);
				}
			}
			txt=XMLGetFirstChild(trustel,"TrustListTrustLevel");
			if(txt)
			{
				if(txt->firstChild())
				{
					std::string tltl=txt->firstChild()->getNodeValue();
					StringFunctions::Convert(tltl,trustlisttrust);
				}
			}
			txt=XMLGetFirstChild(trustel,"MessageTrustComment");
			if(txt)
			{
				if(txt->firstChild())
				{
					messagetrustcomment=SanitizeSingleString(txt->firstChild()->getNodeValue());
				}
			}
			txt=XMLGetFirstChild(trustel,"TrustListTrustComment");
			if(txt)
			{
				if(txt->firstChild())
				{
					trustlisttrustcomment=SanitizeSingleString(txt->firstChild()->getNodeValue());
				}
			}
			if(txt)
			{
				isfms=XMLGetBooleanElement(trustel,"IsFMS");
			}
			if(XMLGetFirstChild(trustel,"IsWOT"))
			{
				iswot=XMLGetBooleanElement(trustel,"IsWOT");
			}

			if(identity!="" && messagetrust>=-1 && messagetrust<=100 && trustlisttrust>=-1 && trustlisttrust<=100)
			{
				// check so we don't add the same identity multiple times from a trust list
				if(std::find(foundkeys.begin(),foundkeys.end(),identity)==foundkeys.end())
				{
					// Trust Lists without version info contain only FMS identities
					if(tlver=="")
					{
						isfms=true;
						iswot=false;
					}
					foundkeys.push_back(identity);
					m_trust.push_back(trust(identity,messagetrust,trustlisttrust,messagetrustcomment,trustlisttrustcomment,isfms,iswot));
				}
			}
			else
			{
				m_log->error("TrustListXML::ParseXML malformed Trust in TrustList.xml");
			}

			trustel=XMLGetNextSibling(trustel,"Trust");
		}

		parsed=true;
	}
	catch(...)
	{
	}

	return parsed;
}
