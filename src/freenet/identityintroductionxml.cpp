#include "../../include/freenet/identityintroductionxml.h"
#include "../../include/freenet/freenetssk.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityIntroductionXML::IdentityIntroductionXML()
{
	Initialize();
}

std::string IdentityIntroductionXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("IdentityIntroduction");

	doc->appendChild(root);

	root->appendChild(XMLCreateCDATAElement(doc,"Identity",m_identity));

	return GenerateXML(doc);
}

void IdentityIntroductionXML::Initialize()
{
	m_identity="";
}

const bool IdentityIntroductionXML::ParseXML(const std::string &xml)
{
	FreenetSSK ssk;
	bool parsed=false;
	Poco::XML::DOMParser dp;

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"IdentityIntroduction");
		Poco::XML::Element *txt=NULL;

		txt=XMLGetFirstChild(root,"Identity");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_identity=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}

		ssk.SetPublicKey(m_identity);
		if(ssk.ValidPublicKey()==false)
		{
			return false;
		}

		parsed=true;
	}
	catch(...)
	{
	}

	return parsed;
}
