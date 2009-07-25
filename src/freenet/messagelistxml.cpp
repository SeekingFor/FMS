#include "../../include/freenet/messagelistxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

MessageListXML::MessageListXML()
{
	Initialize();
}

void MessageListXML::AddMessage(const std::string &date, const long index, const std::vector<std::string> &boards)
{
	m_messages.push_back(message(date,index,boards));
}

void MessageListXML::AddExternalMessage(const std::string &identity, const std::string &date, const long index, const std::vector<std::string> &boards)
{
	m_externalmessages.push_back(externalmessage("Keyed",identity,date,index,boards));
}

void MessageListXML::AddExternalMessage(const std::string &messagekey, const std::string &date, const std::vector<std::string> &boards)
{
	m_externalmessages.push_back(externalmessage("Anonymous",messagekey,date,boards));
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

std::vector<std::string> MessageListXML::GetExternalBoards(const long index)
{
	if(index>=0 && index<m_externalmessages.size())
	{
		return m_externalmessages[index].m_boards;
	}
	else
	{
		return std::vector<std::string>();
	}
}

std::string MessageListXML::GetExternalDate(const long index)
{
	if(index>=0 && index<m_externalmessages.size())
	{
		return m_externalmessages[index].m_date;
	}
	else
	{
		return "";
	}
}

std::string MessageListXML::GetExternalIdentity(const long index)
{
	if(index>=0 && index<m_externalmessages.size())
	{
		return m_externalmessages[index].m_identity;
	}
	else
	{
		return "";
	}
}

const long MessageListXML::GetExternalIndex(const long index)
{
	if(index>=0 && index<m_externalmessages.size())
	{
		return m_externalmessages[index].m_index;
	}
	else
	{
		return-1;
	}
}

std::string MessageListXML::GetExternalMessageKey(const long index)
{
	if(index>=0 && index<m_externalmessages.size())
	{
		return m_externalmessages[index].m_messagekey;
	}
	else
	{
		return "";
	}
}

std::string MessageListXML::GetExternalType(const long index)
{
	if(index>=0 && index<m_externalmessages.size())
	{
		return m_externalmessages[index].m_type;
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
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("MessageList");

	doc->appendChild(root);

	for(std::vector<message>::iterator i=m_messages.begin(); i!=m_messages.end(); i++)
	{
		Poco::AutoPtr<Poco::XML::Element> tr=doc->createElement("Message");
		root->appendChild(tr);
		tr->appendChild(XMLCreateTextElement(doc,"Date",(*i).m_date));
		tr->appendChild(XMLCreateTextElement(doc,"Index",(*i).m_index));
		
		Poco::AutoPtr<Poco::XML::Element> boards=doc->createElement("Boards");
		tr->appendChild(boards);
		for(std::vector<std::string>::iterator j=(*i).m_boards.begin(); j!=(*i).m_boards.end(); j++)
		{
			boards->appendChild(XMLCreateCDATAElement(doc,"Board",(*j)));
		}
	}

	for(std::vector<externalmessage>::iterator i=m_externalmessages.begin(); i!=m_externalmessages.end(); i++)
	{
		Poco::AutoPtr<Poco::XML::Element> tr=doc->createElement("ExternalMessage");
		root->appendChild(tr);
		tr->appendChild(XMLCreateTextElement(doc,"Type",(*i).m_type));
		if((*i).m_type=="Keyed")
		{
			tr->appendChild(XMLCreateCDATAElement(doc,"Identity",(*i).m_identity));
			tr->appendChild(XMLCreateTextElement(doc,"Index",(*i).m_index));
		}
		else
		{
			tr->appendChild(XMLCreateCDATAElement(doc,"MessageKey",(*i).m_messagekey));
		}
		tr->appendChild(XMLCreateTextElement(doc,"Date",(*i).m_date));

		Poco::AutoPtr<Poco::XML::Element> boards=doc->createElement("Boards");
		tr->appendChild(boards);
		for(std::vector<std::string>::iterator j=(*i).m_boards.begin(); j!=(*i).m_boards.end(); j++)
		{
			boards->appendChild(XMLCreateCDATAElement(doc,"Board",(*j)));
		}
	}

	return GenerateXML(doc);
}

void MessageListXML::Initialize()
{
	m_messages.clear();
	m_externalmessages.clear();
}

const bool MessageListXML::ParseXML(const std::string &xml)
{

	bool parsed=false;
	Poco::XML::DOMParser dp;

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"MessageList");

		Poco::XML::Element *node=XMLGetFirstChild(root,"Message");
		while(node)
		{
			std::string date="";
			int index=-1;
			std::vector<std::string> boards;

			Poco::XML::Element *node2=XMLGetFirstChild(node,"Date");
			if(node2 && node2->firstChild())
			{
				date=SanitizeSingleString(node2->firstChild()->getNodeValue());
			}
			node2=XMLGetFirstChild(node,"Index");
			if(node2 && node2->firstChild())
			{
				std::string indexstr=SanitizeSingleString(node2->firstChild()->getNodeValue());
				StringFunctions::Convert(indexstr,index);
			}
			node2=XMLGetFirstChild(node,"Boards");
			if(node2)
			{
				Poco::XML::Element *node3=XMLGetFirstChild(node2,"Board");
				while(node3)
				{
					if(node3 && node3->firstChild())
					{
						std::string boardname=SanitizeSingleString(node3->firstChild()->getNodeValue());
						StringFunctions::LowerCase(boardname,boardname);
						boards.push_back(boardname);
					}
					node3=XMLGetNextSibling(node3,"Board");
				}
			}

			m_messages.push_back(message(date,index,boards));

			node=XMLGetNextSibling(node,"Message");
		}

		node=XMLGetFirstChild(root,"ExternalMessage");
		while(node)
		{
			std::string type="";
			std::string identity="";
			std::string date="";
			int index=-1;
			std::vector<std::string> boards;

			Poco::XML::Element *node2=XMLGetFirstChild(node,"Type");
			if(node2 && node2->firstChild())
			{
				type=SanitizeSingleString(node2->firstChild()->getNodeValue());
			}

			if(type=="Keyed")
			{
				node2=XMLGetFirstChild(node,"Identity");
				if(node2 && node2->firstChild())
				{
					identity=SanitizeSingleString(node2->firstChild()->getNodeValue());
				}
				node2=XMLGetFirstChild(node,"Date");
				if(node2 && node2->firstChild())
				{
					date=SanitizeSingleString(node2->firstChild()->getNodeValue());
				}
				node2=XMLGetFirstChild(node,"Index");
				if(node2 && node2->firstChild())
				{
					std::string indexstr=SanitizeSingleString(node2->firstChild()->getNodeValue());
					StringFunctions::Convert(indexstr,index);
				}
				node2=XMLGetFirstChild(node,"Boards");
				if(node2)
				{
					Poco::XML::Element *node3=XMLGetFirstChild(node2,"Board");
					while(node3)
					{
						if(node3 && node3->firstChild())
						{
							std::string boardname=SanitizeSingleString(node3->firstChild()->getNodeValue());
							StringFunctions::LowerCase(boardname,boardname);
							boards.push_back(boardname);
						}
						node3=XMLGetNextSibling(node3,"Board");
					}
				}
				m_externalmessages.push_back(externalmessage("Keyed",identity,date,index,boards));
			}

			node=XMLGetNextSibling(node,"ExternalMessage");
		}
		parsed=true;
	}
	catch(...)
	{
	}

	return parsed;
}
