#include "../../include/freenet/identityxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityXML::IdentityXML()
{
	Initialize();
}

std::string IdentityXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("Identity");

	doc->appendChild(root);

	root->appendChild(XMLCreateCDATAElement(doc,"Name",m_name));

	if(m_signature!="")
	{
		root->appendChild(XMLCreateCDATAElement(doc,"Signature",m_signature));
	}

	if(m_avatar!="")
	{
		root->appendChild(XMLCreateCDATAElement(doc,"Avatar",m_avatar));
	}

	root->appendChild(XMLCreateBooleanElement(doc,"SingleUse",m_singleuse));

	root->appendChild(XMLCreateBooleanElement(doc,"PublishTrustList",m_publishtrustlist));

	root->appendChild(XMLCreateBooleanElement(doc,"PublishBoardList",m_publishboardlist));

	// freesite edition will be -1 if identity isn't publishing a freesite
	if(m_freesiteedition>=0)
	{
		root->appendChild(XMLCreateTextElement(doc,"FreesiteEdition",m_freesiteedition));
	}

	return GenerateXML(doc);
}

void IdentityXML::Initialize()
{
	m_name="";
	m_publishtrustlist=false;
	m_publishboardlist=false;
	m_singleuse=false;
	m_freesiteedition=-1;
	m_signature="";
	m_avatar="";
}

const bool IdentityXML::ParseXML(const std::string &xml)
{

	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"Identity");
		Poco::XML::Element *txt=NULL;

		txt=XMLGetFirstChild(root,"Name");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_name=txt->firstChild()->getNodeValue();
			}
		}

		txt=XMLGetFirstChild(root,"Signature");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_signature=txt->firstChild()->getNodeValue();
			}
		}

		txt=XMLGetFirstChild(root,"Avatar");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_avatar=txt->firstChild()->getNodeValue();
			}
		}
		
		m_singleuse=XMLGetBooleanElement(root,"SingleUse");
		m_publishtrustlist=XMLGetBooleanElement(root,"PublishTrustList");
		m_publishboardlist=XMLGetBooleanElement(root,"PublishBoardList");
		
		txt=XMLGetFirstChild(root,"FreesiteEdition");
		if(txt)
		{
			if(txt->firstChild())
			{
				std::string editionstr=txt->firstChild()->getNodeValue();
				StringFunctions::Convert(editionstr,m_freesiteedition);
			}
		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;
}
