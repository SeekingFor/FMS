#ifndef _fmsapp_
#define _fmsapp_

#include "threadwrapper/threadedexecutor.h"
#include "ithreaddatabase.h"

#include <Poco/Util/ServerApplication.h>

#include <map>

// main FMS application class
class FMSApp:public Poco::Util::ServerApplication,public IThreadDatabase
{
public:
	FMSApp();

	static void Terminate()	{ ((FMSApp *)&FMSApp::instance())->terminate(); }

private:
	void initialize(Poco::Util::Application &self);
	void initializeLogger();
	void defineOptions(Poco::Util::OptionSet &options);
	int main(const std::vector<std::string> &args);

	void StartThreads();

	void handleHelp(const std::string &name, const std::string &value);
	void displayHelp();
	void handleLogOption(const std::string &name, const std::string &value);
	void handleShowOptions(const std::string &name, const std::string &value);
	void showOptions();
	void handleSetOption(const std::string &name, const std::string &value);
	void setOptions();

	bool m_displayhelp;
	bool m_showoptions;
	bool m_setoption;
	std::map<std::string,std::string> m_setoptions;
	std::string m_logtype;
	std::string m_workingdirectory;

	ThreadedExecutor m_threads;

};

#endif	// _fmsapp_
