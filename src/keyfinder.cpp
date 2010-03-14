#include "../include/keyfinder.h"
#include "../include/stringfunctions.h"

#include <Poco/RegularExpression.h>

KeyFinderItemText::KeyFinderItemText():KeyFinderItem(TYPE_TEXT),m_text("")
{

}

KeyFinderItemText::KeyFinderItemText(const std::string &text):KeyFinderItem(TYPE_TEXT),m_text(text)
{

}

void KeyFinderItemText::Accept(KeyFinderVisitor &visitor)
{
	visitor.Visit(*this);
}

KeyFinderItemKey::KeyFinderItemKey():KeyFinderItem(TYPE_KEY),m_keypart(""),m_filepart("")
{

}

KeyFinderItemKey::KeyFinderItemKey(const std::string &keypart, const std::string &filepart):KeyFinderItem(TYPE_KEY),m_keypart(keypart),m_filepart(filepart)
{

}

void KeyFinderItemKey::Accept(KeyFinderVisitor &visitor)
{
	visitor.Visit(*this);
}

void KeyFinderHTMLRenderVisitor::Visit(KeyFinderItem &item)
{
	if(item.GetItemType()==KeyFinderItem::TYPE_TEXT)
	{
		const KeyFinderItemText *textitem=dynamic_cast<const KeyFinderItemText *>(&item);
		m_rendered+=textitem->GetText();
	}
	else if(item.GetItemType()==KeyFinderItem::TYPE_KEY)
	{
		const KeyFinderItemKey *keyitem=dynamic_cast<const KeyFinderItemKey *>(&item);
		m_rendered+="<a href=\""+m_fproxyprotocol+"://"+m_fproxyhost+":"+m_fproxyport+"/"+keyitem->GetKeyPart()+StringFunctions::UriEncode(keyitem->GetFilePart())+"\">";
		if(keyitem->GetFilePart()=="")
		{
			m_rendered+=keyitem->GetKeyPart();
		}
		m_rendered+=keyitem->GetFilePart();
		m_rendered+="</a>";
	}
}

std::vector<KeyFinderItem *> KeyFinderParser::ParseMessage(const std::string &message)
{
	Poco::RegularExpression keyre("(freenet:)?(CHK@){1}([0-9A-Za-z-~]{43},){2}([0-9A-Za-z-~]){7}");//(([\\S])*(([\\S ])*(\\.[\\S]{3}))?)");
	Poco::RegularExpression filenamere("/([\\S ]*?\\.[\\S]{3})|/([\\S]*)");
	std::vector<std::string::size_type> replacedchars;
	std::string workmessage(message);
	std::vector<KeyFinderItem *> items;
	bool prevnewline=false;
	bool prevblock=false;
	Poco::RegularExpression::Match keymatch;
	Poco::RegularExpression::Match filematch;
	std::vector<Poco::RegularExpression::Match> keymatches;
	std::string::size_type replacedoffset=0;
	bool insidekey=false;
	std::string::size_type currentpos=0;

	// find all characters we are going to ignore - (newlines and the start of block quotes)
	for(std::string::size_type pos=0; pos<message.size(); pos++)
	{
		if(message[pos]=='\r' || message[pos]=='\n')
		{
			replacedchars.push_back(pos);
			prevnewline=true;
			prevblock=false;
		}
		else if((prevnewline==true || prevblock==true) && ((pos+3<message.size()&& message.substr(pos,4)=="&gt;") || message[pos]=='>'))
		{
			replacedchars.push_back(pos);
			prevnewline=false;
			prevblock=true;
		}
		else if(prevblock==true && message[pos]==' ')
		{
			replacedchars.push_back(pos);
			prevnewline=false;
			prevblock=false;
		}
		else
		{
			prevnewline=false;
			prevblock=false;
		}
	}

	// erase all the ignored chars from the message we will be searching
	for(std::vector<std::string::size_type>::reverse_iterator i=replacedchars.rbegin(); i!=replacedchars.rend(); i++)
	{
		workmessage.erase((*i),1);
	}

	// find every key that matches
	keyre.match(workmessage,keymatch);
	while(keymatch.offset!=std::string::npos)
	{
		filenamere.match(workmessage,keymatch.offset+keymatch.length,filematch);
		if(filematch.offset!=std::string::npos && filematch.offset==keymatch.offset+keymatch.length)
		{
			// find the next new line position after the file name
			// we will adjust the length of the filename to continue until the newline if it does not already
			std::string::size_type nextnewlinepos=workmessage.size();
			for(std::string::size_type i=0; i<replacedchars.size(); i++)
			{
				if((replacedchars[i]-i)>filematch.offset)
				{
					nextnewlinepos=replacedchars[i]-i;
					i=replacedchars.size();
				}
			}
			// adjust length of filename to continue until the next newline
			if(nextnewlinepos!=std::string::npos && nextnewlinepos>(filematch.offset+filematch.length))
			{
				filematch.length=nextnewlinepos-filematch.offset;
			}

			keymatch.length+=filematch.length;
		}
		keymatches.push_back(keymatch);
		keyre.match(workmessage,keymatch.offset+keymatch.length,keymatch);
	}

	// put back replaced chars, but not if it is inside a key
	// also adjust positions of keys based on the inserted characters
	for(std::vector<std::string::size_type>::iterator i=replacedchars.begin(); i!=replacedchars.end(); i++)
	{
		insidekey=false;
		for(std::vector<Poco::RegularExpression::Match>::iterator mi=keymatches.begin(); mi!=keymatches.end(); mi++)
		{
			if(((*i)-replacedoffset)<=(*mi).offset && insidekey==false)
			{
				// we're going to insert 1 char - so move the key offset 1 position
				(*mi).offset++;
			}
			else if(((*i)-replacedoffset)>(*mi).offset && ((*i)-replacedoffset)<(*mi).offset+(*mi).length)
			{
				insidekey=true;
				replacedoffset++;
			}
		}
		if(insidekey==false)
		{
			workmessage.insert((*i)-replacedoffset,message.substr((*i),1));
		}
	}

	// add all text and key items to items vector while trying to find filename attached to keys
	currentpos=0;
	for(std::vector<Poco::RegularExpression::Match>::iterator mi=keymatches.begin(); mi!=keymatches.end(); mi++)
	{
		if(currentpos<(*mi).offset)
		{
			items.push_back(new KeyFinderItemText(workmessage.substr(currentpos,(*mi).offset-currentpos)));
		}

		currentpos=(*mi).offset+(*mi).length;

		std::string keypart("");
		std::string filepart("");
		std::string wholekey(workmessage.substr((*mi).offset,(*mi).length));
		std::string::size_type slashpos=wholekey.find("/");
		if(slashpos!=std::string::npos)
		{
			keypart=workmessage.substr((*mi).offset,slashpos+1);
			filepart=workmessage.substr((*mi).offset+slashpos+1,((*mi).length-slashpos)-1);
		}
		else
		{
			keypart=workmessage.substr((*mi).offset,(*mi).length);
		}

		items.push_back(new KeyFinderItemKey(keypart,filepart));

	}

	// push last text part
	if(currentpos<workmessage.size())
	{
		items.push_back(new KeyFinderItemText(workmessage.substr(currentpos)));
	}

	return items;
}

void KeyFinderParser::Cleanup(std::vector<KeyFinderItem *> &items)
{
	for(std::vector<KeyFinderItem *>::const_iterator i=items.begin(); i!=items.end(); i++)
	{
		delete (*i);
	}
	items.clear();
}

std::string KeyFinderHTMLRenderer::Render(const std::string &message, const std::string &fproxyprotocol, const std::string &fproxyhost, const std::string &fproxyport)
{
	KeyFinderHTMLRenderVisitor rv;
	std::vector<KeyFinderItem *> items=KeyFinderParser::ParseMessage(message);
	rv.SetFProxyHost(fproxyhost);
	rv.SetFProxyPort(fproxyport);
	rv.SetFProxyProtocol(fproxyprotocol);

	for(std::vector<KeyFinderItem *>::const_iterator i=items.begin(); i!=items.end(); i++)
	{
		rv.Visit(*(*i));
	}

	KeyFinderParser::Cleanup(items);
	return rv.Rendered();
}
