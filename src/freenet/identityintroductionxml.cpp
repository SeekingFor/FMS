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
	TiXmlDocument td;
	TiXmlDeclaration *tdec=new TiXmlDeclaration("1.0","UTF-8","");
	TiXmlElement *tid;
	TiXmlPrinter tp;

	td.LinkEndChild(tdec);
	tid=new TiXmlElement("IdentityIntroduction");
	td.LinkEndChild(tid);

	tid->LinkEndChild(XMLCreateCDATAElement("Identity",m_identity));

	td.Accept(&tp);
	return std::string(tp.CStr());
}

void IdentityIntroductionXML::Initialize()
{
	m_identity="";
}

const bool IdentityIntroductionXML::ParseXML(const std::string &xml)
{
	FreenetSSK ssk;
	TiXmlDocument td;
	td.Parse(xml.c_str());

	if(!td.Error())
	{
		TiXmlElement *el;
		TiXmlText *txt;
		TiXmlHandle hnd(&td);

		Initialize();

		txt=hnd.FirstChild("IdentityIntroduction").FirstChild("Identity").FirstChild().ToText();
		if(txt)
		{
			m_identity=txt->ValueStr();
		}
		ssk.SetPublicKey(m_identity);
		if(ssk.ValidPublicKey()==false)
		{
			return false;
		}

		return true;

	}
	else
	{
		return false;
	}

}
