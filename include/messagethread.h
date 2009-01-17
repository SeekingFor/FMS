#ifndef _messagethread_
#define _messagethread_

#include "idatabase.h"

class MessageThread:public IDatabase
{
public:
	MessageThread(SQLite3DB::DB *db):IDatabase(db)			{}

	struct threadnode
	{
		long m_messageid;
		long m_level;
		std::string m_subject;
		std::string m_fromname;
		std::string m_date;
	};
	
	void Clear()									{ m_nodes.clear(); }

	const bool Load(const std::string &messageidstr, const long boardid, const bool bydate=false);
	const bool Load(const long messageid, const long boardid, const bool bydate=false);
	
	const std::vector<threadnode> GetNodes()		{ return m_nodes; }
	
private:
	const threadnode GetOriginalMessageNode(const long messageid, const long boardid);
	void AddChildren(const long messageid, const long level, const long boardid);

	class datecompare
	{
	public:
		const bool operator()(const threadnode &node1, const threadnode &node2) const
		{
			return node1.m_date<node2.m_date;
		}
	};

	std::vector<threadnode> m_nodes;
};

#endif	// _messagethread_
