#include "../../include/freenet/sonexml.h"

SoneXML::SoneXML()
{

}

std::string SoneXML::GetXML()
{
	return std::string("");
}

void SoneXML::Initialize()
{
	m_messages.clear();
}

const bool SoneXML::ParseXML(const std::string &xml)
{
	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"sone");
		Poco::XML::Element *posts=NULL;
		Poco::XML::Element *replies=NULL;
		Poco::XML::Element *txt=NULL;

		if(root)
		{
			posts=XMLGetFirstChild(root,"posts");
			replies=XMLGetFirstChild(root,"replies");
		}

		if(posts)
		{

			txt=XMLGetFirstChild(posts,"post");
			while(txt)
			{
				std::string id("");
				std::string timestr("");
				Poco::Timestamp::TimeVal timeval;
				Poco::DateTime messagetime;
				std::string messagetext("");

				Poco::XML::Element *el;
				el=XMLGetFirstChild(txt,"id");
				if(el && el->firstChild())
				{
					id=el->firstChild()->getNodeValue();
				}

				el=XMLGetFirstChild(txt,"time");
				if(el && el->firstChild())
				{
					timestr=el->firstChild()->getNodeValue();
					StringFunctions::Convert(timestr,timeval);
					messagetime=Poco::Timestamp(timeval*static_cast<Poco::Timestamp::TimeVal>(1000));
				}

				el=XMLGetFirstChild(txt,"text");
				if(el && el->firstChild())
				{
					messagetext=el->firstChild()->getNodeValue();
				}

				if(id!="" && messagetext!="")
				{
					m_messages.push_back(message(messagetime,id,std::string(""),messagetext));
				}

				txt=XMLGetNextSibling(txt,"post");
			}
		}

		if(replies)
		{

			txt=XMLGetFirstChild(replies,"reply");
			while(txt)
			{
				std::string id("");
				std::string replyto("");
				std::string timestr("");
				Poco::Timestamp::TimeVal timeval;
				Poco::DateTime messagetime;
				std::string messagetext("");

				Poco::XML::Element *el;
				el=XMLGetFirstChild(txt,"id");
				if(el && el->firstChild())
				{
					id=el->firstChild()->getNodeValue();
				}

				el=XMLGetFirstChild(txt,"post-id");
				if(el && el->firstChild())
				{
					replyto=el->firstChild()->getNodeValue();
				}

				el=XMLGetFirstChild(txt,"time");
				if(el && el->firstChild())
				{
					timestr=el->firstChild()->getNodeValue();
					StringFunctions::Convert(timestr,timeval);
					messagetime=Poco::Timestamp(timeval*static_cast<Poco::Timestamp::TimeVal>(1000));
				}

				el=XMLGetFirstChild(txt,"text");
				if(el && el->firstChild())
				{
					messagetext=el->firstChild()->getNodeValue();
				}

				if(id!="" && messagetext!="")
				{
					m_messages.push_back(message(messagetime,id,replyto,messagetext));
				}

				txt=XMLGetNextSibling(txt,"reply");
			}

		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;

}
