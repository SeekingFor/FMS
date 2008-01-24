#include "../../include/freenet/messagelistxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

MessageListXML::MessageListXML()
{
	Initialize();
}

void MessageListXML::AddMessage(const std::string &date, const long index, const std::vector<std::string> boards)
{
	m_messages.push_back(message(date,index,boards));
}

std::vector<std::string> MessageListXML::GetBoards(const long index)
{
	if(index>=0 && index<m_messages.size())
	{
		return m_messages[index].m_boards;
	}
	else
	{
		return std::vector<std::string>();
	}
}

std::string MessageListXML::GetDate(const long index)
{
	if(index>=0 && index<m_messages.size())
	{
		return m_messages[index].m_date;
	}
	else
	{
		return "";
	}
}

const long MessageListXML::GetIndex(const long index)
{
	if(index>=0 && index<m_messages.size())
	{
		return m_messages[index].m_index;
	}
	else
	{
		return -1;
	}
}

std::string MessageListXML::GetXML()
{
	TiXmlDocument td;
	TiXmlDeclaration *tdec=new TiXmlDeclaration("1.0","UTF-8","");
	TiXmlElement *tid;
	TiXmlPrinter tp;

	td.LinkEndChild(tdec);
	tid=new TiXmlElement("MessageList");
	td.LinkEndChild(tid);

	for(std::vector<message>::iterator i=m_messages.begin(); i!=m_messages.end(); i++)
	{
		TiXmlElement *tr=new TiXmlElement("Message");
		tid->LinkEndChild(tr);
		tr->LinkEndChild(XMLCreateTextElement("Date",(*i).m_date));
		tr->LinkEndChild(XMLCreateTextElement("Index",(*i).m_index));
		TiXmlElement *brds=new TiXmlElement("Boards");
		tr->LinkEndChild(brds);
		for(std::vector<std::string>::iterator j=(*i).m_boards.begin(); j!=(*i).m_boards.end(); j++)
		{
			brds->LinkEndChild(XMLCreateCDATAElement("Board",(*j)));
		}
	}

	td.Accept(&tp);
	return std::string(tp.CStr());
}

void MessageListXML::Initialize()
{
	m_messages.clear();
}

const bool MessageListXML::ParseXML(const std::string &xml)
{
	TiXmlDocument td;
	td.Parse(xml.c_str());

	if(!td.Error())
	{
		std::string tempstr;
		std::string date;
		long index;
		std::vector<std::string> boards;
		TiXmlText *txt;
		TiXmlHandle hnd(&td);
		TiXmlNode *node;
		TiXmlNode *node2;

		Initialize();

		node=hnd.FirstChild("MessageList").FirstChild("Message").ToNode();
		while(node)
		{
			date="";
			index=-1;
			boards.clear();

			TiXmlHandle hnd2(node);
			txt=hnd2.FirstChild("Date").FirstChild().ToText();
			if(txt)
			{
				date=txt->ValueStr();
			}
			txt=hnd2.FirstChild("Index").FirstChild().ToText();
			if(txt)
			{
				tempstr=txt->ValueStr();
				StringFunctions::Convert(tempstr,index);
			}
			node2=hnd2.FirstChild("Boards").FirstChild("Board").ToNode();
			while(node2)
			{
				if(node2->FirstChild())
				{
					boards.push_back(node2->FirstChild()->ValueStr());
				}
				node2=node2->NextSibling("Board");
			}

			m_messages.push_back(message(date,index,boards));

			node=node->NextSibling("Message");
		}

		return true;
	}
	else
	{
		return false;
	}
}
