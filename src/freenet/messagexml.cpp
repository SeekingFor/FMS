#include "../../include/freenet/messagexml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

MessageXML::MessageXML()
{
	Initialize();
}

std::string MessageXML::GetXML()
{
	TiXmlDocument td;
	TiXmlDeclaration *tdec=new TiXmlDeclaration("1.0","UTF-8","");
	TiXmlElement *tid;
	TiXmlPrinter tp;

	td.LinkEndChild(tdec);
	tid=new TiXmlElement("Message");
	td.LinkEndChild(tid);

	tid->LinkEndChild(XMLCreateTextElement("Date",m_date));
	tid->LinkEndChild(XMLCreateTextElement("Time",m_time));
	tid->LinkEndChild(XMLCreateCDATAElement("Subject",m_subject));
	tid->LinkEndChild(XMLCreateCDATAElement("MessageID",m_messageid));
	tid->LinkEndChild(XMLCreateCDATAElement("ReplyBoard",m_replyboard));
	tid->LinkEndChild(XMLCreateCDATAElement("Body",m_body));

	TiXmlElement *brds=new TiXmlElement("Boards");
	tid->LinkEndChild(brds);
	for(std::vector<std::string>::iterator i=m_boards.begin(); i!=m_boards.end(); i++)
	{
		brds->LinkEndChild(XMLCreateCDATAElement("Board",(*i)));
	}

	if(m_inreplyto.size()>0)
	{
		TiXmlElement *rply=new TiXmlElement("InReplyTo");
		tid->LinkEndChild(rply);
		for(std::map<long,std::string>::iterator j=m_inreplyto.begin(); j!=m_inreplyto.end(); j++)
		{
			TiXmlElement *mess=new TiXmlElement("Message");
			rply->LinkEndChild(mess);
			mess->LinkEndChild(XMLCreateTextElement("Order",(*j).first));
			mess->LinkEndChild(XMLCreateCDATAElement("MessageID",(*j).second));
		}
	}

	td.Accept(&tp);
	return std::string(tp.CStr());
}

void MessageXML::Initialize()
{
	m_date="";
	m_time="";
	m_subject="";
	m_boards.clear();
	m_replyboard="";
	m_inreplyto.clear();
	m_body="";
}

const bool MessageXML::ParseXML(const std::string &xml)
{
	TiXmlDocument td;
	td.Parse(xml.c_str());

	if(!td.Error())
	{
		TiXmlHandle hnd(&td);
		TiXmlNode *node1;
		TiXmlNode *node2;
		TiXmlText *txt;

		Initialize();

		txt=hnd.FirstChild("Message").FirstChild("Date").FirstChild().ToText();
		if(txt)
		{
			m_date=txt->ValueStr();
		}
		txt=hnd.FirstChild("Message").FirstChild("Time").FirstChild().ToText();
		if(txt)
		{
			m_time=txt->ValueStr();
		}
		txt=hnd.FirstChild("Message").FirstChild("Subject").FirstChild().ToText();
		if(txt)
		{
			m_subject=txt->ValueStr();
		}
		txt=hnd.FirstChild("Message").FirstChild("MessageID").FirstChild().ToText();
		if(txt)
		{
			m_messageid=txt->ValueStr();
		}
		txt=hnd.FirstChild("Message").FirstChild("ReplyBoard").FirstChild().ToText();
		if(txt)
		{
			m_replyboard=txt->ValueStr();
		}
		txt=hnd.FirstChild("Message").FirstChild("Body").FirstChild().ToText();
		if(txt)
		{
			m_body=txt->ValueStr();
		}

		node2=hnd.FirstChild("Message").FirstChild("Boards").FirstChild("Board").ToNode();
		while(node2)
		{
			if(node2->FirstChild())
			{
				m_boards.push_back(node2->FirstChild()->ValueStr());
			}
			node2=node2->NextSibling("Board");
		}

		node2=hnd.FirstChild("Message").FirstChild("InReplyTo").FirstChild("Message").ToNode();
		while(node2)
		{
			std::string orderstr;
			long order=-1;
			std::string messageid="";
			TiXmlHandle hnd2(node2);
			txt=hnd2.FirstChild("Order").FirstChild().ToText();
			if(txt)
			{
				orderstr=txt->ValueStr();
				StringFunctions::Convert(orderstr,order);
			}
			txt=hnd2.FirstChild("MessageID").FirstChild().ToText();
			if(txt)
			{
				messageid=txt->ValueStr();
			}

			if(order!=-1 && messageid!="")
			{
				m_inreplyto[order]=messageid;
			}

			node2=node2->NextSibling("Message");
		}

		return true;
	}
	else
	{
		return false;
	}
}
