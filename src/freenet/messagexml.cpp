#include "../../include/freenet/messagexml.h"
#include "../../include/board.h"

#ifdef XMEM
	#include <xmem.h>
#endif

MessageXML::MessageXML()
{
	Initialize();
}

std::string MessageXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("Message");

	doc->appendChild(root);

	root->appendChild(XMLCreateTextElement(doc,"Date",m_date));
	root->appendChild(XMLCreateTextElement(doc,"Time",m_time));
	root->appendChild(XMLCreateCDATAElement(doc,"Subject",m_subject));
	root->appendChild(XMLCreateCDATAElement(doc,"MessageID",m_messageid));
	root->appendChild(XMLCreateCDATAElement(doc,"ReplyBoard",m_replyboard));
	
	root->appendChild(XMLCreateCDATAElement(doc,"Body",m_body));

	Poco::AutoPtr<Poco::XML::Element> brds=doc->createElement("Boards");

	root->appendChild(brds);

	// attach boards
	for(std::vector<std::string>::iterator i=m_boards.begin(); i!=m_boards.end(); i++)
	{
		std::string boardname=(*i);
		StringFunctions::Convert(boardname,boardname);
		brds->appendChild(XMLCreateCDATAElement(doc,"Board",boardname));
	}

	// attach inreplyto ids
	if(m_inreplyto.size()>0)
	{
		Poco::AutoPtr<Poco::XML::Element> rply=doc->createElement("InReplyTo");
		root->appendChild(rply);
		for(std::map<long,std::string>::iterator j=m_inreplyto.begin(); j!=m_inreplyto.end(); j++)
		{
			Poco::AutoPtr<Poco::XML::Element> mess=doc->createElement("Message");
			rply->appendChild(mess);
			mess->appendChild(XMLCreateTextElement(doc,"Order",(*j).first));
			mess->appendChild(XMLCreateCDATAElement(doc,"MessageID",(*j).second));
		}
	}

	// add attachemnt node if we have attachments
	if(m_fileattachments.size()>0)
	{
		Poco::AutoPtr<Poco::XML::Element> attachments=doc->createElement("Attachments");
		root->appendChild(attachments);
		for(std::vector<fileattachment>::iterator j=m_fileattachments.begin(); j!=m_fileattachments.end(); j++)
		{
			Poco::AutoPtr<Poco::XML::Element> f=doc->createElement("File");
			attachments->appendChild(f);
			f->appendChild(XMLCreateCDATAElement(doc,"Key",(*j).m_key));
			f->appendChild(XMLCreateTextElement(doc,"Size",(*j).m_size));
		}
	}

	return GenerateXML(doc,false);
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
	m_fileattachments.clear();
}

const bool MessageXML::ParseXML(const std::string &xml)
{
	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"Message");
		Poco::XML::Element *txt=NULL;

		txt=XMLGetFirstChild(root,"Date");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_date=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}
		txt=XMLGetFirstChild(root,"Time");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_time=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}
		txt=XMLGetFirstChild(root,"Subject");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_subject=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}
		txt=XMLGetFirstChild(root,"MessageID");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_messageid=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}
		}
		txt=XMLGetFirstChild(root,"ReplyBoard");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_replyboard=SanitizeSingleString(txt->firstChild()->getNodeValue());
				// strip off everything after , in board name
				if(m_replyboard.find(",")!=std::string::npos)
				{
					m_replyboard.erase(m_replyboard.find(","));
				}
				m_replyboard=Board::FixBoardName(m_replyboard);
			}
		}
		txt=XMLGetFirstChild(root,"Body");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_body=txt->firstChild()->getNodeValue();
			}
		}
		Poco::XML::Element *boards=XMLGetFirstChild(root,"Boards");
		if(boards)
		{
			Poco::XML::Element *board=XMLGetFirstChild(boards,"Board");
			while(board)
			{
				if(board->firstChild())
				{
					std::string boardname=SanitizeSingleString(board->firstChild()->getNodeValue());
					// strip off everything after , in board name
					if(boardname.find(",")!=std::string::npos)
					{
						boardname.erase(boardname.find(","));
					}
					boardname=Board::FixBoardName(boardname);
					m_boards.push_back(boardname);
				}
				board=XMLGetNextSibling(board,"Board");
			}
		}
		Poco::XML::Element *inreplyto=XMLGetFirstChild(root,"InReplyTo");
		if(inreplyto)
		{
			Poco::XML::Element *message=XMLGetFirstChild(inreplyto,"Message");
			while(message)
			{
				Poco::XML::Element *orderel=XMLGetFirstChild(message,"Order");
				Poco::XML::Element *messageidel=XMLGetFirstChild(message,"MessageID");
				if(orderel && orderel->firstChild() && messageidel && messageidel->firstChild())
				{
					int order=-1;
					std::string messageid="";

					StringFunctions::Convert(orderel->firstChild()->getNodeValue(),order);
					messageid=messageidel->firstChild()->getNodeValue();

					if(order!=-1 && messageid!="")
					{
						m_inreplyto[order]=messageid;
					}
				}
				message=XMLGetNextSibling(message,"Message");
			}
		}
		Poco::XML::Element *attachments=XMLGetFirstChild(root,"Attachments");
		if(attachments)
		{
			Poco::XML::Element *file=XMLGetFirstChild(attachments,"File");
			while(file)
			{
				Poco::XML::Element *keyel=XMLGetFirstChild(file,"Key");
				Poco::XML::Element *sizeel=XMLGetFirstChild(file,"Size");

				if(keyel && keyel->firstChild() && sizeel && sizeel->firstChild())
				{
					int size=-1;
					std::string key="";
					
					StringFunctions::Convert(sizeel->firstChild()->getNodeValue(),size);
					key=keyel->firstChild()->getNodeValue();

					if(size!=-1 && key!="")
					{
						m_fileattachments.push_back(fileattachment(key,size));
					}
				}

				file=XMLGetNextSibling(file,"File");
			}
		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;
}
