#include "../../include/freenet/fmsversionxml.h"

FMSVersionXML::FMSVersionXML()
{
	Initialize();
}

std::string FMSVersionXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("FMSVersion");

	doc->appendChild(root);

	root->appendChild(XMLCreateTextElement(doc,"Major",m_major));

	root->appendChild(XMLCreateTextElement(doc,"Minor",m_minor));

	root->appendChild(XMLCreateTextElement(doc,"Release",m_release));

	root->appendChild(XMLCreateCDATAElement(doc,"Notes",m_notes));

	root->appendChild(XMLCreateCDATAElement(doc,"Changes",m_changes));

	root->appendChild(XMLCreateCDATAElement(doc,"PageKey",m_pagekey));

	root->appendChild(XMLCreateCDATAElement(doc,"SourceKey",m_sourcekey));

	return GenerateXML(doc);
}

void FMSVersionXML::Initialize()
{
	m_major=0;
	m_minor=0;
	m_release=0;
	m_notes="";
	m_changes="";
	m_pagekey="";
	m_sourcekey="";
}

const bool FMSVersionXML::ParseXML(const std::string &xml)
{

	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"FMSVersion");
		Poco::XML::Element *txt=NULL;

		txt=XMLGetFirstChild(root,"Major");
		if(txt && txt->firstChild())
		{
			std::string tempstr=txt->firstChild()->getNodeValue();
			StringFunctions::Convert(tempstr,m_major);
		}
		txt=XMLGetFirstChild(root,"Minor");
		if(txt && txt->firstChild())
		{
			std::string tempstr=txt->firstChild()->getNodeValue();
			StringFunctions::Convert(tempstr,m_minor);
		}
		txt=XMLGetFirstChild(root,"Release");
		if(txt && txt->firstChild())
		{
			std::string tempstr=txt->firstChild()->getNodeValue();
			StringFunctions::Convert(tempstr,m_release);
		}
		txt=XMLGetFirstChild(root,"Notes");
		if(txt && txt->firstChild())
		{
			m_notes=txt->firstChild()->getNodeValue();
		}
		txt=XMLGetFirstChild(root,"Changes");
		if(txt && txt->firstChild())
		{
			m_changes=txt->firstChild()->getNodeValue();
		}
		txt=XMLGetFirstChild(root,"PageKey");
		if(txt && txt->firstChild())
		{
			m_pagekey=txt->firstChild()->getNodeValue();
		}
		txt=XMLGetFirstChild(root,"SourceKey");
		if(txt && txt->firstChild())
		{
			m_sourcekey=txt->firstChild()->getNodeValue();
		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;
}
