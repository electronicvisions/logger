// Singleton implementation of Debugger class for symap2ic project
// by Daniel Bruederle, bruederle@kip.uni-heidelberg.de
// by Sebastian Jeltsch, sjeltsch@kip.uni-heidelberg.de
//
// IMMER LOGGER BLEIBEN!
//
// 2010-03-01


#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstddef>
#include <boost/shared_ptr.hpp>

//! Singleton implementation of Debugger class
/*! Only one single instance of this class can be created by calling the public function instance(). 
  Every further call returns a reference to this one class. The parameters loglevel and logfile can 
  not be changed after their first initialization. 
  Pre-defined logging levels are: ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3.
  Default log level ist DEBUG if you reference the the debugger wit no explicit log level */
class Logger
{
	private:
		size_t loglevel;
		static boost::shared_ptr<Logger> log_ptr;
		std::string logfilename;
		std::fstream* logfile;
		std::ostringstream deafStream;

		Logger(size_t level, std::string filename);
		Logger(Logger&);

		std::string getTime();
		std::string toString(size_t level);
		std::string toColor(size_t level);
		const std::string resetColor() const;

		std::ostream& getFileStream(size_t level);
		std::ostream& getStdStream(size_t level);

	public:
		enum levels {ERROR=0, WARNING=1, INFO=2, DEBUG=3, DEBUG1=4, DEBUG2=5, DEBUG3=6};

		~Logger();

		/*! This is the only way to create an instance of a Logger. Only the first call actually creates an instance, 
		  all further calls returns a reference to the one and only instance. */
		static Logger& instance(
				size_t level=WARNING,       //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
				std::string filename=""     //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
				);

		size_t getLevel();
		std::string getFilename();

		//! tells if the passed logging level is sufficient to pass the the logger's criticality threshold
		bool willBeLogged(unsigned int level);

		std::ostream& operator() (size_t level=DEBUG);

		std::ostream& operator<<(std::basic_string<char> val);
		std::ostream& operator<<(std::ostringstream& val);
		std::ostream& operator<<(long val);
		std::ostream& operator<<(unsigned long val);
		std::ostream& operator<<(bool val);
		std::ostream& operator<<(short val);
		std::ostream& operator<<(unsigned short val);
		std::ostream& operator<<(int val);
		std::ostream& operator<<(unsigned int val);
		std::ostream& operator<<(long long val);
		std::ostream& operator<<(unsigned long long val);
		std::ostream& operator<<(double val);
		std::ostream& operator<<(float val);
		std::ostream& operator<<(long double val);
		std::ostream& operator<<(const char* val);
		std::ostream& operator<<(const void* val);
};


Logger::Logger(size_t level, std::string filename)
{
	logfile = 0;
	loglevel = level;
	logfilename = filename;
}

Logger::Logger(Logger&) {}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)

#include <windows.h>

inline std::string Logger::getTime()
{
	const int MAX_LEN = 200;
	char buffer[MAX_LEN];
	if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
				"HH':'mm':'ss", buffer, MAX_LEN) == 0)
		return "Error in NowTime()";

	char result[100] = {0};
	static DWORD first = GetTickCount();
	std::sprintf(result, "%s.%03ld ", buffer, (long)(GetTickCount() - first) % 1000);
	return result;
}

#else

#include <sys/time.h>

inline std::string Logger::getTime()
{
	char buffer[11];
	time_t t;
	time(&t);
	tm r = {0};
	strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld ", buffer, (long)tv.tv_usec / 1000);
	return result;
}

#endif //WIN32

inline std::string Logger::toString(size_t level)
{
	static const char* const buffer[] = { "ERROR: ",
	                                      "WARNING: ",
										  "INFO: ",
										  "DEBUG: ",
										  "DEBUG1: ",
										  "DEBUG2: ",
										  "DEBUG3: "};
	return buffer[level];
										  //"\33[32mDEBUG1:\t",
}

inline std::string Logger::toColor(size_t level)
{
	switch(level) {
		case ERROR:
			return std::string("\33[31m");
		case INFO:
			return std::string("\33[33m");
		default:
			return std::string("\33[32m");
	}
}

inline const std::string Logger::resetColor() const
{
	return std::string("\33[0m");
}

inline std::ostream& Logger::getFileStream(size_t level)
{
	*logfile << getTime();
	logfile->width(10);
	*logfile << std::left << toString(level);
	return *logfile;
}

inline std::ostream& Logger::getStdStream(size_t level)
{
	std::cout << getTime() << toColor(level);
	std::cout.width(10);
	std::cout << std::left << toString(level) << resetColor();
	return std::cout;
}

Logger::~Logger()
{
	if(logfile)
	{
		if(logfile->is_open())
		{
			logfile->flush();
			logfile->close();
		}
		delete logfile;
	}
}

Logger::Logger& Logger::instance(
		size_t level,             //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
		std::string filename      //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
		)
{
	if(!log_ptr) log_ptr = boost::shared_ptr<Logger> (new Logger(level, filename));
	return *log_ptr;
}

inline size_t Logger::getLevel()
{
	return loglevel;
}

inline std::string Logger::getFilename()
{
	return logfilename;
}

inline bool Logger::willBeLogged(unsigned int level)
{
	return (level <= loglevel);
}

std::ostream& Logger::operator() (size_t level)
{
	if(willBeLogged(level))
	{
		if(!logfile)
		{
			if(logfilename=="") return getStdStream(level);
			else
			{
				// erase existing log file if necessary
				logfile = new std::fstream;
				logfile->open(logfilename.c_str(), (std::fstream::out | std::fstream::binary) );
				*logfile << "";
				logfile->close();

				// open clean log file in append mode
				logfile->open(logfilename.c_str(), (std::fstream::app | std::fstream::out | std::fstream::binary) );
			}
		}
		return getFileStream(level);
	}
	else
	{
		// return deaf stream
		deafStream.clear();
		return deafStream;
	}
}

inline std::ostream& Logger::operator<<(std::basic_string<char> val)    { return (*this)() << val; }
inline std::ostream& Logger::operator<<(std::ostringstream& val)        { return (*this)() << val; }
inline std::ostream& Logger::operator<<(long val)                       { return (*this)() << val; }
inline std::ostream& Logger::operator<<(unsigned long val)              { return (*this)() << val; }
inline std::ostream& Logger::operator<<(bool val)                       { return (*this)() << val; }
inline std::ostream& Logger::operator<<(short val)                      { return (*this)() << val; }
inline std::ostream& Logger::operator<<(unsigned short val)             { return (*this)() << val; }
inline std::ostream& Logger::operator<<(int val)                        { return (*this)() << val; }
inline std::ostream& Logger::operator<<(unsigned int val)               { return (*this)() << val; }
inline std::ostream& Logger::operator<<(long long val)                  { return (*this)() << val; }
inline std::ostream& Logger::operator<<(unsigned long long val)         { return (*this)() << val; }
inline std::ostream& Logger::operator<<(double val)                     { return (*this)() << val; }
inline std::ostream& Logger::operator<<(float val)                      { return (*this)() << val; }
inline std::ostream& Logger::operator<<(long double val)                { return (*this)() << val; }
inline std::ostream& Logger::operator<<(const char* val)                { return (*this)() << val; }
inline std::ostream& Logger::operator<<(const void* val)                { return (*this)() << val; }


#ifndef PYTHONEXPOSED
// Allocating and initializing Logger's static data member.  
// The smart pointer is allocated - not the object inself.
boost::shared_ptr<Logger> Logger::log_ptr;
#endif //PYTHONEXPOSED

#endif // __LOGGER_H__
