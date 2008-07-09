#include "../../include/http/identityexportxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityExportXML::IdentityExportXML()
{
	Initialize();
}

void IdentityExportXML::AddIdentity(const std::string &name, const std::string &publickey, const std::string &privatekey, const bool singleuse, const bool publishtrustlist, const bool publishboardlist, const bool publishfreesite)
{
	m_identities.push_back(identity(name,publickey,privatekey,singleuse,publishtrustlist,publishboardlist,publishfreesite));
}

const std::string IdentityExportXML::GetName(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_identities[index].m_name;
	}
	else
	{
		return "";
	}
}

const std::string IdentityExportXML::GetPrivateKey(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_identities[index].m_privatekey;
	}
	else
	{
		return "";
	}
}

const std::string IdentityExportXML::GetPublicKey(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_identities[index].m_publickey;
	}
	else
	{
		return "";
	}
}

const bool IdentityExportXML::GetPublishBoardList(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_identities[index].m_publishboardlist;
	}
	else
	{
		return false;
	}
}

const bool IdentityExportXML::GetPublishFreesite(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_identities[index].m_publishfreesite;
	}
	else
	{
		return false;
	}
}

const bool IdentityExportXML::GetPublishTrustList(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_identities[index].m_publishtrustlist;
	}
	else
	{
		return false;
	}
}

const bool IdentityExportXML::GetSingleUse(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_identities[index].m_singleuse;
	}
	else
	{
		return false;
	}
}

std::string IdentityExportXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("IdentityExport");
	Poco::AutoPtr<Poco::XML::Element> el=NULL;

	doc->appendChild(root);

	for(std::vector<identity>::iterator i=m_identities.begin(); i!=m_identities.end(); i++)
	{
		el=doc->createElement("Identity");
		root->appendChild(el);

		el->appendChild(XMLCreateCDATAElement(doc,"Name",(*i).m_name));
		el->appendChild(XMLCreateTextElement(doc,"PublicKey",(*i).m_publickey));
		el->appendChild(XMLCreateTextElement(doc,"PrivateKey",(*i).m_privatekey));
		el->appendChild(XMLCreateBooleanElement(doc,"SingleUse",(*i).m_singleuse));
		el->appendChild(XMLCreateBooleanElement(doc,"PublishTrustList",(*i).m_publishtrustlist));
		el->appendChild(XMLCreateBooleanElement(doc,"PublishBoardList",(*i).m_publishboardlist));
		el->appendChild(XMLCreateBooleanElement(doc,"PublishFreesite",(*i).m_publishfreesite));
	}

	return GenerateXML(doc);
}

void IdentityExportXML::Initialize()
{
	m_identities.clear();
}

const bool IdentityExportXML::ParseXML(const std::string &xml)
{
	bool parsed=false;
	Poco::XML::DOMParser dp;

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"IdentityExport");
		Poco::XML::Element *node=XMLGetFirstChild(root,"Identity");

		while(node)
		{
			std::string name="";
			std::string publickey="";
			std::string privatekey="";
			bool singleuse=false;
			bool publishtrustlist=false;
			bool publishboardlist=false;
			bool publishfreesite=false;	

			Poco::XML::Element *text=XMLGetFirstChild(node,"Name");
			if(text)
			{
				if(text->firstChild())
				{
					std::string asdf=text->innerText();
					asdf=text->firstChild()->innerText();
					name=text->firstChild()->getNodeValue();
				}
			}
			text=XMLGetFirstChild(node,"PublicKey");
			if(text)
			{
				if(text->firstChild())
				{
					publickey=text->firstChild()->getNodeValue();
				}
			}
			text=XMLGetFirstChild(node,"PrivateKey");
			if(text)
			{
				if(text->firstChild())
				{
					privatekey=text->firstChild()->getNodeValue();
				}
			}

			singleuse=XMLGetBooleanElement(node,"SingleUse");
			publishtrustlist=XMLGetBooleanElement(node,"PublishTrustList");
			publishboardlist=XMLGetBooleanElement(node,"PublishBoardList");
			publishfreesite=XMLGetBooleanElement(node,"PublishFreesite");

			if(name!="" && publickey!="" && privatekey!="")
			{
				m_identities.push_back(identity(name,publickey,privatekey,singleuse,publishtrustlist,publishboardlist,publishfreesite));
			}

			node=XMLGetNextSibling(node,"Identity");
		}

		parsed=true;
	}
	catch(...)
	{
	}

	return parsed;
}
