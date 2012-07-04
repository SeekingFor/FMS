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
	m_avatar="";
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
		Poco::XML::Element *profile=NULL;
		Poco::XML::Element *albums=NULL;

		if(root)
		{
			posts=XMLGetFirstChild(root,"posts");
			replies=XMLGetFirstChild(root,"replies");
			profile=XMLGetFirstChild(root,"profile");
			albums=XMLGetFirstChild(root,"albums");
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

		if(profile)
		{
			std::string avatarid("");
			Poco::XML::Element *avatar=XMLGetFirstChild(profile,"avatar");
			if(avatar && avatar->firstChild() && avatar->firstChild()->getNodeValue()!="")
			{
				std::string avatarid=avatar->firstChild()->getNodeValue();
				if(albums)
				{
					Poco::XML::Element *album=XMLGetFirstChild(albums,"album");
					while(album)
					{
						Poco::XML::Element *images=XMLGetFirstChild(album,"images");
						if(images)
						{
							Poco::XML::Element *image=XMLGetFirstChild(images,"image");
							while(image)
							{
								Poco::XML::Element *imid=XMLGetFirstChild(image,"id");
								if(imid && imid->firstChild())
								{
									if(imid->firstChild()->getNodeValue()==avatarid)
									{
										Poco::XML::Element *key=XMLGetFirstChild(image,"key");
										if(key && key->firstChild())
										{
											m_avatar=key->firstChild()->getNodeValue();
										}
									}
								}
								image=XMLGetNextSibling(image,"image");
							}
						}
						album=XMLGetNextSibling(album,"album");
					}
				}
			}
		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;

}
