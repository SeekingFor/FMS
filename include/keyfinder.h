#ifndef _keyfinder_
#define _keyfinder_

#include <string>
#include <vector>

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
	virtual void Visit(KeyFinderItem &item);

	void SetFProxyHost(const std::string &host)	{ m_fproxyhost=host; }
	void SetFProxyPort(const std::string &port) { m_fproxyport=port; }

private:
	std::string m_fproxyhost;
	std::string m_fproxyport;
};

class KeyFinderParser
{
public:
	static std::vector<KeyFinderItem *> ParseMessage(const std::string &message);
	static void Cleanup(std::vector<KeyFinderItem *> &items);
};

class KeyFinderHTMLRenderer
{
public:
	static std::string Render(const std::string &message, const std::string &fproxyhost, const std::string &fproxyport);
};

#endif	// _keyfinder_
