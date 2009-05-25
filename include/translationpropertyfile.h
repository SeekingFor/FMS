#ifndef _translationpropertyfile_
#define _translationpropertyfile_

#include <Poco/Util/MapConfiguration.h>

/*
	Duplicate most of Poco::Util::PropertyFileConfiguration because it can't handle UTF-8 properly
*/
class TranslationPropertyFile:public Poco::Util::MapConfiguration
{
public:
	TranslationPropertyFile()		{}
	~TranslationPropertyFile()		{}

	void load(std::istream& istr);
		/// Loads the configuration data from the given stream, which 
		/// must be in properties file format.

	void load(const std::string& path);
		/// Loads the configuration data from the given file, which 
		/// must be in properties file format.

	void save(std::ostream& ostr) const;
		/// Writes the configuration data to the given stream.
		///
		/// The data is written as a sequence of statements in the form
		/// <key>= <value>
		/// separated by a newline character.

	void save(const std::string& path) const;
		/// Writes the configuration data to the given file.

	void GetKeys(std::vector<std::string> &resultkeys) const;

private:
	void GetSubKeys(const std::string &prefix, std::vector<std::string> &resultkeys) const;
	void parseLine(std::istream& istr);
	static int readChar(std::istream& istr);

	template <class S>
	S trim(const S& str)
		/// Returns a copy of str with all leading and
		/// trailing whitespace removed.
	{
		int first = 0;
		int last  = int(str.size()) - 1;
		
		while (first <= last && std::isspace((unsigned char)str[first])) ++first;
		while (last >= first && std::isspace((unsigned char)str[last])) --last;

		return S(str, first, last - first + 1);
	}
};

#endif	// _translationpropertyfile_
