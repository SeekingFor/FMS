#ifndef _quoter_
#define _quoter_

#include <vector>
#include <string>

class QuoterVisitor;

class QuoterItem
{
public:
	QuoterItem();
	QuoterItem(const int itemtype, const int level=0);
	virtual ~QuoterItem()				{ }

	virtual void Accept(QuoterVisitor &visitor)=0;

	void SetLevel(const int level)		{ m_level=level; }
	const int GetLevel() const			{ return m_level; }

	const int GetItemType() const		{ return m_itemtype; }

	enum ItemType
	{
		TYPE_UNKNOWN=0,
		TYPE_TEXT=1,
		TYPE_AREA=2
	};

private:
	int m_level;
	int m_itemtype;
};

class QuoterItemText:public QuoterItem
{
public:
	QuoterItemText();
	QuoterItemText(const std::string &text, const int level);

	virtual void Accept(QuoterVisitor &visitor);

	void SetText(const std::string &text)	{ m_text=text; }
	const std::string &GetText() const		{ return m_text; }

private:
	std::string m_text;
};

class QuoterItemArea:public QuoterItem
{
public:
	QuoterItemArea();
	QuoterItemArea(const int level);
	~QuoterItemArea();

	virtual void Accept(QuoterVisitor &visitor);

	void AddItem(QuoterItem *item);
	const std::vector<QuoterItem *> &GetItems() const	{ return m_items; }

private:
	std::vector<QuoterItem *> m_items;
};

class QuoterVisitor
{
public:
	virtual ~QuoterVisitor()				{ }
	virtual void Visit(const QuoterItem &item)=0;
};

class QuoterRenderVisitor:public QuoterVisitor
{
public:
	virtual ~QuoterRenderVisitor()			{ }
	const std::string &Rendered() const		{ return m_rendered; }
protected:
	std::string m_rendered;
};

class QuoterHTMLRenderVisitor:public QuoterRenderVisitor
{
public:
	virtual void Visit(const QuoterItem &item);
};

class QuoterParser
{
public:
	static QuoterItem *ParseMessage(const std::string &message);
	static void Cleanup(QuoterItem *item);
private:
	static QuoterItem *ParseArea(const std::string &block, const int level);
};

class QuoterHTMLRenderer
{
public:
	static std::string Render(const std::string &message);
};

#endif	// _quoter_
