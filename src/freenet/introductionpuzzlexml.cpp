#include "../../include/freenet/introductionpuzzlexml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IntroductionPuzzleXML::IntroductionPuzzleXML()
{
	Initialize();
}

std::string IntroductionPuzzleXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("IntroductionPuzzle");

	doc->appendChild(root);

	root->appendChild(XMLCreateTextElement(doc,"Type",m_type));

	root->appendChild(XMLCreateCDATAElement(doc,"UUID",m_uuid));

	root->appendChild(XMLCreateTextElement(doc,"MimeType",m_mimetype));

	root->appendChild(XMLCreateTextElement(doc,"PuzzleData",m_puzzledata));

	return GenerateXML(doc);
}

void IntroductionPuzzleXML::Initialize()
{
	m_type="";
	m_uuid="";
	m_puzzledata="";
	m_mimetype="";
}

const bool IntroductionPuzzleXML::ParseXML(const std::string &xml)
{
	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"IntroductionPuzzle");
		Poco::XML::Element *txt=NULL;

		txt=XMLGetFirstChild(root,"Type");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_type=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}
		txt=XMLGetFirstChild(root,"UUID");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_uuid=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}
		txt=XMLGetFirstChild(root,"MimeType");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_mimetype=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}
		txt=XMLGetFirstChild(root,"PuzzleData");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_puzzledata=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}

		parsed=true;
	}
	catch(...)
	{
	}

	return parsed;
}
