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
	TiXmlDocument td;
	TiXmlDeclaration *tdec=new TiXmlDeclaration("1.0","UTF-8","");
	TiXmlElement *tid;
	TiXmlPrinter tp;

	td.LinkEndChild(tdec);
	tid=new TiXmlElement("IntroductionPuzzle");
	td.LinkEndChild(tid);

	tid->LinkEndChild(XMLCreateTextElement("Type",m_type));

	tid->LinkEndChild(XMLCreateTextElement("UUID",m_uuid));

	tid->LinkEndChild(XMLCreateTextElement("MimeType",m_mimetype));

	tid->LinkEndChild(XMLCreateTextElement("PuzzleData",m_puzzledata));

	td.Accept(&tp);
	return std::string(tp.CStr());
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
	TiXmlDocument td;
	td.Parse(xml.c_str());

	if(!td.Error())
	{
		TiXmlElement *el;
		TiXmlText *txt;
		TiXmlHandle hnd(&td);

		Initialize();

		txt=hnd.FirstChild("IntroductionPuzzle").FirstChild("Type").FirstChild().ToText();
		if(txt)
		{
			m_type=txt->ValueStr();
		}

		txt=hnd.FirstChild("IntroductionPuzzle").FirstChild("UUID").FirstChild().ToText();
		if(txt)
		{
			m_uuid=txt->ValueStr();
		}

		txt=hnd.FirstChild("IntroductionPuzzle").FirstChild("MimeType").FirstChild().ToText();
		if(txt)
		{
			m_mimetype=txt->ValueStr();
		}

		txt=hnd.FirstChild("IntroductionPuzzle").FirstChild("PuzzleData").FirstChild().ToText();
		if(txt)
		{
			m_puzzledata=txt->ValueStr();
		}

		return true;

	}
	else
	{
		return false;
	}
}