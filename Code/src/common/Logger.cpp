#include "Logger.h"
#include <assert.h>
#include <time.h>
#include <sstream>

Logger* Logger::m_pLogger = new Logger();

Logger::Logger(void)
{
}

Logger::~Logger(void)
{
	m_file.close();
	delete m_file;
}

void Logger::configure(string filename)
{
	m_file.open(filename, fstream::in | fstream::app);
}

void Logger::log(LogSeverity lvl, string message)
{
	assert(m_file);

	stringstream output;

	// set line prefix
	if(lvl == logInfo)
	{
		output << "INFO\t";
	}
	else if(lvl == logWarning)
	{
		output << "WARNING\t";
	}
	else
	{
		output << "ERROR\t";
	}

	// write the current time in the line
	time_t t = time(0);
	struct tm * now = localtime(&t);
	output << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << " " << now->tm_mday << "." << (now->tm_mon + 1) << "." << (now->tm_year + 1900) << "\t";

	// append the message
	output << message << "\n";
	
	m_streamWriteMutex._Lock();
	m_file << output.str();
	m_file.flush();
	m_streamWriteMutex._Unlock();
}