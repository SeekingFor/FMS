#include "../include/fmsapp.h"
#include "../include/global.h"
#include "../include/dbsetup.h"
#include "../include/optionssetup.h"
#include "../include/option.h"
#include "../include/stringfunctions.h"
#include "../include/http/httpthread.h"
#include "../include/nntp/nntplistener.h"
#include "../include/dbmaintenancethread.h"
#include "../include/freenet/freenetmasterthread.h"
#include "../include/threadwrapper/threadedexecutor.h"
#include "../include/db/sqlite3db.h"

#include <Poco/Util/HelpFormatter.h>
#include <Poco/FileChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
	#include <direct.h>
#else
	#include <unistd.h>
#endif

FMSApp::FMSApp():m_displayhelp(false),m_showoptions(false),m_setoption(false),m_logtype("file"),m_workingdirectory("")
{
	// get current working dir so we can go to it later
	char wd[1024];
	char *wdptr=NULL;
	memset(wd,0,1024);
	wdptr=getcwd(wd,1023);
	if(wdptr)
	{
		m_workingdirectory=wdptr;
	}
}

void FMSApp::defineOptions(Poco::Util::OptionSet &options)
{
	ServerApplication::defineOptions(options);

	// add command line options here
	options.addOption(Poco::Util::Option("help","?","Display help for command line arguments.",false).repeatable(false).callback(Poco::Util::OptionCallback<FMSApp>(this,&FMSApp::handleHelp)));
	options.addOption(Poco::Util::Option("log","l","Select type of log output (file|stdout|stderr).",false,"type",true).repeatable(false).callback(Poco::Util::OptionCallback<FMSApp>(this,&FMSApp::handleLogOption)));
	options.addOption(Poco::Util::Option("showoptions","","Show all options that can be set and their current values.",false).repeatable(false).callback(Poco::Util::OptionCallback<FMSApp>(this,&FMSApp::handleShowOptions)));
	options.addOption(Poco::Util::Option("setoption","","Set an option.  Values are not validated, so be sure to set them correctly.",false,"option=value",true).repeatable(true).callback(Poco::Util::OptionCallback<FMSApp>(this,&FMSApp::handleSetOption)));
}

void FMSApp::displayHelp()
{
	Poco::Util::HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");
	helpFormatter.setHeader("The Freenet Message System.");
	helpFormatter.format(std::cout);
}

void FMSApp::handleHelp(const std::string &name, const std::string &value)
{
	m_displayhelp=true;
	displayHelp();
	stopOptionsProcessing();
}

void FMSApp::handleLogOption(const std::string &name, const std::string &value)
{
	if(value=="file" || value=="stdout" || value=="stderr")
	{
		m_logtype=value;
	}
}

void FMSApp::handleSetOption(const std::string &name, const std::string &value)
{
	std::vector<std::string> valueparts;
	StringFunctions::Split(value,"=",valueparts);

	if(valueparts.size()==2)
	{
		m_setoptions[valueparts[0]]=valueparts[1];
	}
	else
	{
		std::cout << "Expected option=value but found " << value << std::endl;
	}

	m_setoption=true;
}

void FMSApp::handleShowOptions(const std::string &name, const std::string &value)
{
	m_showoptions=true;
}

void FMSApp::initialize(Poco::Util::Application &self)
{
	ServerApplication::initialize(self);

	// set working directory - fall back on application.dir if working directory isn't set
	// if we are runing as a service, then working directory needs to be set to the application directory
	if(m_workingdirectory=="" || config().getBool("application.runAsService",false)==true)
	{
		m_workingdirectory=config().getString("application.dir");
	}
	int rval=chdir(m_workingdirectory.c_str());

	SetupDB();
	SetupDefaultOptions();
	initializeLogger();
	config().setString("application.logger","logfile");
}

void FMSApp::initializeLogger()
{
	int initiallevel=Poco::Message::PRIO_TRACE;

	std::string tempval="";
	if(Option::Instance()->Get("LogLevel",tempval))
	{
		StringFunctions::Convert(tempval,initiallevel);
	}

	Poco::AutoPtr<Poco::FormattingChannel> formatter=new Poco::FormattingChannel(new Poco::PatternFormatter("%Y-%m-%d %H:%M:%S | %p | %t"));
	
	if(m_logtype=="file")
	{
		Poco::AutoPtr<Poco::FileChannel> fc=new Poco::FileChannel("fms.log");
		fc->setProperty("rotation","daily");	// rotate log file daily
		fc->setProperty("times","utc");			// utc date/times for log entries
		fc->setProperty("archive","timestamp");	// add timestamp to old logs
		fc->setProperty("purgeCount","30");		// purge old logs after 30 logs have accumulated
		fc->setProperty("compress","true");		// gz compress old log files
		formatter->setChannel(fc);
	}
	else
	{
		if(m_logtype=="stdout")
		{
			Poco::AutoPtr<Poco::ConsoleChannel> cc=new Poco::ConsoleChannel(std::cout);
			formatter->setChannel(cc);
		}
		else
		{
			Poco::AutoPtr<Poco::ConsoleChannel> cc=new Poco::ConsoleChannel(std::cerr);
			formatter->setChannel(cc);
		}
	}
	
	setLogger(Poco::Logger::create("logfile",formatter,Poco::Message::PRIO_INFORMATION));
	Poco::Logger::get("logfile").information("LogLevel set to "+tempval);
	Poco::Logger::get("logfile").setLevel(initiallevel);
}

int FMSApp::main(const std::vector<std::string> &args)
{

	// running as a daemon would reset the working directory to / before calling main
	// so we need to set the working directory again
	int rval=chdir(m_workingdirectory.c_str());

	if(m_displayhelp)
	{
	}
	else if(m_showoptions)
	{
		showOptions();
	}
	else if(m_setoption)
	{
		setOptions();
	}
	else
	{
		logger().information("FMS startup v"FMS_VERSION);

		std::string tempval="";
		Option::Instance()->Get("VacuumOnStartup",tempval);
		if(tempval=="true")
		{
			logger().information("VACUUMing database");
			SQLite3DB::DB::Instance()->Execute("VACUUM;");
		}

		StartThreads();

		if(isInteractive()==true)
		{
			std::cout << "FMS has been started." << std::endl << std::endl;
		}

		waitForTerminationRequest();

		if(isInteractive()==true)
		{
			std::cout << "Please wait while FMS shuts down." << std::endl << std::endl;
		}

		logger().trace("FMSApp::main cancelling threads");
		m_threads.Cancel();
		logger().trace("FMSApp::main joining threads");
		m_threads.Join();

		logger().information("FMS shutdown");
	}

	return FMSApp::EXIT_OK;
}

void FMSApp::setOptions()
{
	for(std::map<std::string,std::string>::iterator i=m_setoptions.begin(); i!=m_setoptions.end(); i++)
	{
		std::string tempval="";
		if(Option::Instance()->Get((*i).first,tempval))
		{
			Option::Instance()->Set((*i).first,(*i).second);
			std::cout << "Option " << (*i).first << " set to " << (*i).second << std::endl;
		}
		else
		{
			std::cout << "Unknown option " << (*i).first << std::endl;
		}
	}
}

void FMSApp::showOptions()
{
	SQLite3DB::Statement st=SQLite3DB::DB::Instance()->Prepare("SELECT Option, OptionValue FROM tblOption;");
	st.Step();
	while(st.RowReturned())
	{
		std::string option="";
		std::string optionvalue="";
		st.ResultText(0,option);
		st.ResultText(1,optionvalue);

		std::cout << option << " = " << optionvalue << std::endl;

		st.Step();
	}
}

void FMSApp::StartThreads()
{
	std::string tempval="";

	// always start the DB maintenance thread
	logger().trace("FMSApp::StartThreads starting DBMaintenanceThread");
	m_threads.Start(new DBMaintenanceThread());

	Option::Instance()->Get("StartHTTP",tempval);
	if(tempval=="true")
	{
		logger().trace("FMSApp::StartThreads starting HTTPThread");
		m_threads.Start(new HTTPThread());
	}
	else
	{
		logger().trace("FMSApp::StartThreads not starting HTTPThread");
	}

	tempval="";
	Option::Instance()->Get("StartNNTP",tempval);
	if(tempval=="true")
	{
		logger().trace("FMSApp::StartThreads starting NNTPListener");
		m_threads.Start(new NNTPListener());
	}
	else
	{
		logger().trace("FMSApp::StartThreads not starting NNTPListener");
	}

	tempval="";
	Option::Instance()->Get("StartFreenetUpdater",tempval);
	if(tempval=="true")
	{
		logger().trace("FMSApp::StartThreads starting FreenetMasterThread");
		m_threads.Start(new FreenetMasterThread());
	}
	else
	{
		logger().trace("FMSApp::StartThreads not starting FreenetMasterThread");
	}

}
