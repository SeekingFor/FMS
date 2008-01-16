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
	TiXmlDocument td;
	TiXmlDeclaration *tdec=new TiXmlDeclaration("1.0","UTF-8","");
	TiXmlElement *tid;
	TiXmlPrinter tp;

	td.LinkEndChild(tdec);
	tid=new TiXmlElement("Identity");
	td.LinkEndChild(tid);

	tid->LinkEndChild(XMLCreateCDATAElement("Name",m_name));

	tid->LinkEndChild(XMLCreateBooleanElement("SingleUse",m_singleuse));

	tid->LinkEndChild(XMLCreateBooleanElement("PublishTrustList",m_publishtrustlist));

	tid->LinkEndChild(XMLCreateBooleanElement("PublishBoardList",m_publishboardlist));

	td.Accept(&tp);
	return std::string(tp.CStr());

}

void IdentityXML::Initialize()
{
	m_name="";
	m_publishtrustlist=false;
	m_publishboardlist=false;
	m_singleuse=false;
}

const bool IdentityXML::ParseXML(const std::string &xml)
{
	TiXmlDocument td;
	td.Parse(xml.c_str());

	if(!td.Error())
	{
		TiXmlElement *el;
		TiXmlText *txt;
		TiXmlHandle hnd(&td);

		Initialize();

		txt=hnd.FirstChild("Identity").FirstChild("Name").FirstChild().ToText();
		if(txt)
		{
			m_name=txt->ValueStr();
		}

		m_singleuse=XMLGetBooleanElement(hnd.FirstChild("Identity").ToElement(),"SingleUse");

		m_publishtrustlist=XMLGetBooleanElement(hnd.FirstChild("Identity").ToElement(),"PublishTrustList");

		m_publishboardlist=XMLGetBooleanElement(hnd.FirstChild("Identity").ToElement(),"PublishBoardList");

		return true;

	}
	else
	{
		return false;
	}

}
