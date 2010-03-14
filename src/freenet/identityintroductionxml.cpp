#include "../../include/freenet/identityintroductionxml.h"
#include "../../include/freenet/freenetkeys.h"

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
	FreenetSSKKey ssk;
	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

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
				if(ssk.TryParse(m_identity)==false)
				{
					return false;
				}
				m_identity=ssk.GetBaseKey();
			}
		}

		parsed=true;
	}
	catch(...)
	{
	}

	return parsed;
}
