#ifndef _queryvar_
#define _queryvar_

class QueryVar
{
public:
	QueryVar():m_name(""),m_data(""),m_contenttype(""),m_filename("")															{}
	QueryVar(const std::string &name, const std::string &data):m_name(name),m_data(data),m_contenttype(""),m_filename("")		{}

	const std::string &GetName() const			{ return m_name; }
	const std::string &GetData() const			{ return m_data; }
	const std::string &GetContentType() const	{ return m_contenttype; }
	const std::string &GetFileName() const		{ return m_filename; }

	void SetName(const std::string &name)					{ m_name=name; }
	void SetData(const std::string &data)					{ m_data=data; }
	void SetContentType(const std::string &contenttype)		{ m_contenttype=contenttype; }
	void SetFileName(const std::string &filename)			{ m_filename=filename; }

	const bool operator==(const std::string &rhs) const	{ return m_data==rhs; }
	const bool operator==(const QueryVar &rhs) const	{ return m_data==rhs.m_data; }
	const bool operator!=(const std::string &rhs) const	{ return m_data!=rhs; }
	const bool operator!=(const QueryVar &rhs) const	{ return m_data!=rhs.m_data; }

private:
	std::string m_name;
	std::string m_data;
	std::string m_contenttype;
	std::string m_filename;
};

#endif	// _queryvar_
