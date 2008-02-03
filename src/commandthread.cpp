#include "../include/commandthread.h"
#include "../include/stringfunctions.h"

#include <iostream>

#ifdef XMEM
	#include <xmem.h>
#endif

void CommandThread::HandleHelpCommand()
{
	std::cout << "Available Commands:" << std::endl;
	std::cout << "QUIT              End program" << std::endl;
}

void CommandThread::HandleInput(const std::string &input)
{
	std::string command=input;
	std::string argument="";
	if(input.find(" ")!=std::string::npos)
	{
		command=input.substr(0,input.find(" "));
		argument=input.substr(command.size()+1);
	}
	StringFunctions::UpperCase(command,command);
	
	if(command=="HELP")
	{
		HandleHelpCommand();
	}
	else if(command=="QUIT")
	{
		HandleQuit();
	}
	else
	{
		std::cout << "Unknown command.  Type HELP for a list of available commands." << std::endl;
	}
	
}

void CommandThread::HandleQuit()
{
	m_running=false;	
}

void CommandThread::Run()
{
	std::string input;
	m_running=true;

	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"CommandThread::run thread started.");
	
	do
	{

		std::cout << ">";
		std::cin >> input;
		
		HandleInput(input);
		
	}while(m_running && !IsCancelled());
	
}
