#ifndef _keyfinder_
#define _keyfinder_

#include "http/emoticonreplacer.h"

#include <string>
#include <vector>

#include <Poco/RegularExpression.h>

class KeyFinderVisitor;

class KeyFinderItem
{
public:
	KeyFinderItem(const int itemtype=TYPE_UNKNOWN)	{ m_itemtype=itemtype; }
	virtual ~KeyFinderItem()						{ }

	virtual void Accept(KeyFinderVisitor &visitor)=0;

	const int GetItemType() const					{ return m_itemtype; }

	enum ItemType
	{
		TYPE_UNKNOWN=0,
		TYPE_TEXT=1,
		TYPE_KEY=2
	};

private:
	int m_itemtype;
};

class KeyFinderItemText:public KeyFinderItem
{
public:
	KeyFinderItemText();
	KeyFinderItemText(const std::string &text);

	virtual void Accept(KeyFinderVisitor &visitor);

	void SetText(const std::string &text)		{ m_text=text; }
	const std::string &GetText() const			{ return m_text; }

private:
	std::string m_text;
};

class KeyFinderItemKey:public KeyFinderItem
{
public:
	KeyFinderItemKey();
	KeyFinderItemKey(const std::string &keypart, const std::string &filepart);

	virtual void Accept(KeyFinderVisitor &visitor);

	const std::string &GetKeyPart() const		{ return m_keypart; }
	const std::string &GetFilePart() const		{ return m_filepart; }

private:
	std::string m_keypart;
	std::string m_filepart;
};

class KeyFinderVisitor
{
public:
	virtual ~KeyFinderVisitor()			{ }
	virtual void Visit(KeyFinderItem &item)=0;
};

class KeyFinderRenderVisitor:public KeyFinderVisitor
{
public:
	virtual ~KeyFinderRenderVisitor()	{ }
	const std::string &Rendered() const	{ return m_rendered; }
protected:
	std::string m_rendered;
};

class KeyFinderHTMLRenderVisitor:public KeyFinderRenderVisitor
{
public:
	KeyFinderHTMLRenderVisitor()	{ }
	virtual void Visit(KeyFinderItem &item);

	void SetFProxyHost(const std::string &host)				{ m_fproxyhost=host; }
	void SetFProxyPort(const std::string &port)				{ m_fproxyport=port; }
	void SetFProxyProtocol(const std::string &protocol)		{ m_fproxyprotocol=protocol; }
	void SetShowSmilies(const bool showsmilies)				{ m_showsmilies=showsmilies; }
	void SetEmoticonReplacer(EmoticonReplacer *emot)		{ m_emot=emot; }

private:
	std::string m_fproxyhost;
	std::string m_fproxyport;
	std::string m_fproxyprotocol;
	bool m_showsmilies;
	EmoticonReplacer *m_emot;
};

class KeyFinderParser
{
public:
	KeyFinderParser();
	std::vector<KeyFinderItem *> ParseMessage(const std::string &message);
	void Cleanup(std::vector<KeyFinderItem *> &items);
private:
	Poco::RegularExpression m_keyre;
	Poco::RegularExpression m_protocolre;
	Poco::RegularExpression m_filenamere;
};

class KeyFinderHTMLRenderer
{
public:
	std::string Render(const std::string &message, const std::string &fproxyprotocol, const std::string &fproxyhost, const std::string &fproxyport, const bool showsmilies, EmoticonReplacer *emot);
private:
	KeyFinderParser m_parser;
};

#endif	// _keyfinder_
