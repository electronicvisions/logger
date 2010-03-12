#include "logger.h"

#ifdef MULTI_THREAD
Logger::Logger(size_t level, std::string filename) : local_stream(&del_local_stream), logfilename(filename), loglevel(level)
#else
Logger::Logger(size_t level, std::string filename) : local_stream(NULL), logfilename(filename), loglevel(level)
#endif // MULTI_THREAD
{
	deafstream.setstate(std::ostream::badbit);
	std::ostream* tmp = &std::cout;

	if (logfilename != "")
	{
		logfile = new std::ofstream;
		logfile->open(logfilename.c_str(), (std::fstream::out | std::fstream::binary) );
		if (!logfile->is_open())
			throw std::runtime_error("Logger ERROR: unable to open given logfile");
		tmp=logfile;
	}
	*tmp << "*** Started logging @" << getTime() << " ***" << std::endl;
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
		return "Error in Logger::getTime()";

	char result[100] = {0};
	static DWORD first = GetTickCount();
	std::sprintf(result, "%s.%03ld ", buffer, (long)(GetTickCount() - first) % 1000);
	return result;
}

#else

#include <sys/time.h>

inline std::string Logger::getTime()
{
	char buffer[20];
	time_t t;
	time(&t);
	tm r;
	strftime(buffer, sizeof(buffer), "%x %X", localtime_r(&t, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld ", buffer, (long)tv.tv_usec / 1000);
	return result;
}

#endif //WIN32

inline std::string Logger::toString(size_t level)
{
	return buffer[level];
}

inline const char* Logger::toColor(size_t level) const
{
	switch(level) {
		case ERROR:
			return COLOR_ERROR;
		case WARNING:
			return COLOR_WARNING;
		default:
			return COLOR_DEFAULT;
	}
}

inline const char* Logger::resetColor() const
{
	return COLOR_RESET;
}

inline std::ostream& Logger::formatStream(size_t level)
{
	*local_stream << getTime();
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	if (!logfile) *local_stream << toColor(level);
#endif //WIN32
	local_stream->width(10);
	*local_stream << std::left << toString(level);
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	if (!logfile) *local_stream << resetColor();
#endif //WIN32
	return *local_stream;
}

inline std::ostream& Logger::getOutStream()
{
	if (logfile) return *logfile;
	return std::cout;
}

Logger::~Logger()
{
#ifndef MULTI_THREAD
	getOutStream() << local_stream->str();
	delete local_stream;
#else
	local_stream.reset( NULL );
#endif // MULTI_THREAD
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
#ifdef MULTI_THREAD
	boost::mutex::scoped_lock lock(init_mutex);
#endif // MULTI_THREAD
	if(!log_ptr)
	{
		if ( level > DEBUG3 ) level = DEBUG3;
		log_ptr = boost::shared_ptr<Logger> (new Logger(level, filename));
	}
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

inline bool Logger::willBeLogged(size_t level)
{
	return (level <= loglevel);
}

std::ostream& Logger::operator() (size_t level)
{
	if(willBeLogged(level))
	{
#ifdef MULTI_THREAD
		local_stream.reset( new std::ostringstream );
#else
		if (local_stream)
		{
			*local_stream << std::endl;
			getOutStream() << local_stream->str();
		}
		delete local_stream;
		local_stream = new std::ostringstream;
#endif // MULTI_THREAD
		return formatStream(level);
	}
	else return deafstream;
}

const char* const Logger::buffer[] = {
	"ERROR: ", "WARNING: ", "INFO: ",
	"DEBUG0: ", "DEBUG1: ", "DEBUG2: ",
	"DEBUG3: " };

// Allocating and initializing Logger's static data member.  
// The smart pointer is allocated - not the object itself.
boost::shared_ptr<Logger> Logger::log_ptr;
std::ofstream* Logger::logfile(NULL);

#ifdef MULTI_THREAD
boost::mutex Logger::init_mutex;

inline void del_local_stream( std::ostringstream* stream )
{
	Logger::getOutStream() << stream->str() << std::endl;
	delete stream;
}
#endif // MULTI_THREAD
