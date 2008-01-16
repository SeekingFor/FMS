#ifndef _ifmsxmldocument_
#define _ifmsxmldocument_

#include <string>
#include <tinyxml.h>

/**
	\brief Interface for objects that represent an XML document
*/
class IFMSXMLDocument
{
public:
	
	/**
		\brief Returns xml document represented by this object
		
		\return xml document
	*/
	virtual std::string GetXML()=0;
	
	/**
		\brief Parses an xml document into this object
		
		\return true if the document was parsed successfully, false if it was not
	*/
	virtual const bool ParseXML(const std::string &xml)=0;

protected:
	/**
		\brief Creates and returns an element with a boolean value
	*/
	virtual TiXmlElement *XMLCreateBooleanElement(const std::string &name, const bool value)
	{
		TiXmlText *txt=new TiXmlText(value ? "true" : "false");
		TiXmlElement *el=new TiXmlElement(name);
		el->LinkEndChild(txt);
		return el;
	}

	/**
		\brief Creates and returns an element with a CDATA value
	*/
	virtual TiXmlElement *XMLCreateCDATAElement(const std::string &name, const std::string &data)
	{
		TiXmlText *txt=new TiXmlText(data);
		txt->SetCDATA(true);
		TiXmlElement *el=new TiXmlElement(name);
		el->LinkEndChild(txt);
		return el;
	}

	/**
		\brief Creates and returns a text element
	*/
	virtual TiXmlElement *XMLCreateTextElement(const std::string &name, const std::string &data)
	{
		TiXmlText *txt=new TiXmlText(data);
		TiXmlElement *el=new TiXmlElement(name);
		el->LinkEndChild(txt);
		return el;
	}

	virtual const bool XMLGetBooleanElement(TiXmlElement *parent, const std::string &name)
	{
		TiXmlHandle hnd(parent);
		TiXmlText *txt=hnd.FirstChild(name).FirstChild().ToText();
		if(txt)
		{
			if(txt->ValueStr()=="true")
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		return false;
	}
	
};

#endif	// _ifmsxmldocument_
