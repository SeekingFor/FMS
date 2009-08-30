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

void TrustListXML::AddTrust(const std::string &identity, const long messagetrust, const long trustlisttrust, const std::string &messagetrustcomment, const std::string &trustlisttrustcomment)
{
	m_trust.push_back(trust(identity,messagetrust,trustlisttrust,messagetrustcomment,trustlisttrustcomment));
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

std::string TrustListXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("TrustList");

	doc->appendChild(root);

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

		std::vector<std::string> foundkeys;

		trustel=XMLGetFirstChild(root,"Trust");
		while(trustel)
		{
			std::string identity="";
			int messagetrust=-1;
			int trustlisttrust=-1;
			std::string messagetrustcomment="";
			std::string trustlisttrustcomment="";

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

			if(identity!="" && messagetrust>=-1 && messagetrust<=100 && trustlisttrust>=-1 && trustlisttrust<=100)
			{
				// check so we don't add the same identity multiple times from a trust list
				if(std::find(foundkeys.begin(),foundkeys.end(),identity)==foundkeys.end())
				{
					foundkeys.push_back(identity);
					m_trust.push_back(trust(identity,messagetrust,trustlisttrust,messagetrustcomment,trustlisttrustcomment));
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
