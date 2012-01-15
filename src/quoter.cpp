#include "../include/quoter.h"
#include "../include/stringfunctions.h"

#include <sstream>
#include <algorithm>

QuoterItem::QuoterItem():m_level(0),m_itemtype(TYPE_UNKNOWN)
{

}

QuoterItem::QuoterItem(const int itemtype, const int level):m_level(level),m_itemtype(itemtype)
{

}

QuoterItemText::QuoterItemText():QuoterItem(TYPE_TEXT),m_text("")
{

}

QuoterItemText::QuoterItemText(const std::string &text, const int level):QuoterItem(TYPE_TEXT,level),m_text(text)
{
	// remove spaces from the beginnings of lines
	std::string::size_type pos=m_text.find("\n ");
	std::string::size_type notpos=0;
	std::string::size_type nextpos=0;
	while(pos!=std::string::npos)
	{
		if(pos+2<m_text.size())
		{
			nextpos=m_text.find("\n ",pos+1);
			notpos=m_text.find_first_not_of(" ",pos+1);
			if(notpos!=std::string::npos && notpos<nextpos)
			{
				while(pos+1<m_text.size() && m_text[pos+1]==' ')
				{
					m_text.erase(pos+1,1);
				}
			}
		}

		pos=m_text.find("\n ",pos+1);
	}
}

void QuoterItemText::Accept(QuoterVisitor &visitor)
{
	visitor.Visit(*this);
}

const std::string QuoterItemText::GetSanitizedText(bool showsmilies, EmoticonReplacer *emot) const
{
	std::string output(m_text);
	output=StringFunctions::Replace(output,"\r\n","\n");
	output=StringFunctions::Replace(output,"&","&amp;");
	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"[","&#91;");
	output=StringFunctions::Replace(output,"]","&#93;");

	if(showsmilies==true)
	{
		output=emot->Replace(output);
	}

	output=StringFunctions::Replace(output,"\n","<br />");

	return output;
}

QuoterItemArea::QuoterItemArea():QuoterItem(TYPE_AREA),m_items()
{

}

QuoterItemArea::QuoterItemArea(const int level):QuoterItem(TYPE_AREA,level),m_items()
{

}

QuoterItemArea::~QuoterItemArea()
{
	for(std::vector<QuoterItem *>::const_iterator i=m_items.begin(); i!=m_items.end(); i++)
	{
		delete (*i);
	}
	m_items.clear();
}

void QuoterItemArea::Accept(QuoterVisitor &visitor)
{
	visitor.Visit(*this);
}

void QuoterItemArea::AddItem(QuoterItem *item)
{
	m_items.push_back(item);
}

void QuoterHTMLRenderVisitor::Visit(const QuoterItem &item)
{
	if(item.GetItemType()==QuoterItem::TYPE_TEXT)
	{
		const QuoterItemText *textitem=dynamic_cast<const QuoterItemText *>(&item);
		if(m_detectlinks==true)
		{
			m_rendered+=m_keyrenderer.Render(textitem->GetText(),"[FPROXYPROTOCOL]","[FPROXYHOST]","[FPROXYPORT]",m_showsmilies,m_emot);
		}
		else
		{
			m_rendered+=textitem->GetSanitizedText(m_showsmilies,m_emot);
		}
	}
	else if(item.GetItemType()==QuoterItem::TYPE_AREA)
	{
		const QuoterItemArea *textarea=dynamic_cast<const QuoterItemArea *>(&item);
		if(textarea->GetLevel()>0)
		{
			std::ostringstream levelstr("");
			levelstr << (std::max)((std::min)(textarea->GetLevel(),8),1);

			m_rendered+="<div class=\"forumquote quotelevel"+levelstr.str()+"\">";
		}
		for(std::vector<QuoterItem *>::const_iterator i=textarea->GetItems().begin(); i!=textarea->GetItems().end(); i++)
		{
			(*i)->Accept(*this);
		}
		if(textarea->GetLevel()>0)
		{
			m_rendered+="</div>";
		}
	}
}

QuoterItem *QuoterParser::ParseMessage(const std::string &message)
{
	return ParseArea(message,0);
}

void QuoterParser::Cleanup(QuoterItem *item)
{
	delete item;
}

QuoterItem *QuoterParser::ParseArea(const std::string &block, const int level)
{
	std::string currentblocktext("");
	std::string::size_type currentpos=0;
	std::string::size_type endlinepos=0;
	int currentlevel=0;
	int lastlevel=0;
	QuoterItemArea *result=new QuoterItemArea(level);

	while(currentpos<block.size())
	{
		endlinepos=block.find("\n",currentpos);
		if(endlinepos==std::string::npos)
		{
			endlinepos=block.size();
		}

		currentlevel=0;

		if(currentpos<endlinepos && (block[currentpos]=='>' || block.find("&gt;",currentpos)==currentpos))
		{
			if(block[currentpos]=='>')
			{
				currentpos++;
			}
			else
			{
				currentpos+=4;
			}
			currentlevel++;
		}

		if(currentlevel!=lastlevel)
		{
			if(lastlevel==0)
			{
				result->AddItem(new QuoterItemText(currentblocktext,level));
			}
			else
			{
				result->AddItem(ParseArea(currentblocktext,level+1));
			}
			currentblocktext="";
		}

		currentblocktext+=block.substr(currentpos,(endlinepos-currentpos)+1);

		lastlevel=currentlevel;
		currentpos=endlinepos+1;
	}

	// add last block
	if(lastlevel==0)
	{
		result->AddItem(new QuoterItemText(currentblocktext,level));
	}
	else
	{
		result->AddItem(ParseArea(currentblocktext,level+1));
	}

	return result;
}

std::string QuoterHTMLRenderer::Render(const std::string &message)
{
	QuoterItem *item=QuoterParser::ParseMessage(message);

	m_rv.Clear();
	m_rv.Visit(*item);
	std::string rval=m_rv.Rendered();

	QuoterParser::Cleanup(item);

	return rval;
}
