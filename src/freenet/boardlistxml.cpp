#include "../../include/freenet/boardlistxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

BoardListXML::BoardListXML()
{
	Initialize();
}

void BoardListXML::AddBoard(const std::string &name, const std::string &description)
{
	if(name!="" && description!="")
	{
		std::string lowername=name;
		StringFunctions::LowerCase(lowername,lowername);
		m_boards.push_back(board(lowername,description));
	}
}

const std::string BoardListXML::GetDescription(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_boards[index].m_description;
	}
	else
	{
		return "";
	}
}

const std::string BoardListXML::GetName(const long index)
{
	if(index>=0 && index<GetCount())
	{
		return m_boards[index].m_name;
	}
	else
	{
		return "";
	}
}

std::string BoardListXML::GetXML()
{
	Poco::AutoPtr<Poco::XML::Document> doc=new Poco::XML::Document;
	Poco::AutoPtr<Poco::XML::Element> root=doc->createElement("BoardList");

	doc->appendChild(root);

	for(std::vector<board>::iterator i=m_boards.begin(); i!=m_boards.end(); i++)
	{
		std::string boardname=(*i).m_name;
		StringFunctions::LowerCase(boardname,boardname);
		Poco::AutoPtr<Poco::XML::Element> tr=doc->createElement("Board");
		root->appendChild(tr);
		tr->appendChild(XMLCreateCDATAElement(doc,"Name",boardname));
		tr->appendChild(XMLCreateCDATAElement(doc,"Description",(*i).m_description));
	}

	return GenerateXML(doc);
}

void BoardListXML::Initialize()
{
	m_boards.clear();
}

const bool BoardListXML::ParseXML(const std::string &xml)
{
	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"BoardList");
		Poco::XML::Element *boardel=NULL;

		boardel=XMLGetFirstChild(root,"Board");
		while(boardel)
		{
			std::string name="";
			std::string description="";

			Poco::XML::Element *txt=XMLGetFirstChild(boardel,"Name");
			if(txt && txt->firstChild())
			{
				name=SanitizeSingleString(txt->firstChild()->getNodeValue());
				StringFunctions::LowerCase(name,name);
			}
			txt=XMLGetFirstChild(boardel,"Description");
			if(txt && txt->firstChild())
			{
				description=SanitizeSingleString(txt->firstChild()->getNodeValue());
			}

			if(name!="" && description!="")
			{
				m_boards.push_back(board(name,description));
			}

			boardel=XMLGetNextSibling(boardel,"Board");
		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;
}
