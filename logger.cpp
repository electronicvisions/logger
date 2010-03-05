#include "logger.h"
#include <cassert>

Logger::Logger(size_t level, std::string filename) : logfile(NULL), logfilename(filename), loglevel(level)
{
	deafstream.setstate(std::ostream::badbit);

	if (filename != "")
	{
		// erase existing log file if necessary
		logfile = new std::fstream;
		logfile->open(filename.c_str(), (std::fstream::out | std::fstream::binary) );
		assert(logfile->is_open());
		*logfile << "";
		logfile->close();

		// open clean log file in append mode
		logfile->open(filename.c_str(), (std::fstream::app | std::fstream::out | std::fstream::binary) );
		assert(logfile->is_open());
	}
	std::cout << "*** Started logging @" << getTime() << " ***" << std::endl;
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
	tm r;
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

inline std::ostream& Logger::getStream(std::ostream& stream, size_t level, bool color)
{
	stream << getTime();
	if (color) stream << toColor(level);
	stream.width(10);
	stream << std::left << toString(level);
	if (color) stream << resetColor();
	return stream;
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
	boost::mutex::scoped_lock lock(init_mutex);
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

inline bool Logger::willBeLogged(unsigned int level)
{
	return (level <= loglevel);
}

std::ostream& Logger::operator() (size_t level)
{
	if(willBeLogged(level))
	{
		return logfile ? getStream(*logfile, level) : getStream(std::cout, level, true);
	}
	else return deafstream;
}

const char* const Logger::buffer[] = {
	"ERROR: ", "WARNING: ", "INFO: ",
	"DEBUG: ", "DEBUG1: ", "DEBUG2: ",
	"DEBUG3: " };

// Allocating and initializing Logger's static data member.  
// The smart pointer is allocated - not the object inself.
boost::shared_ptr<Logger> Logger::log_ptr;
boost::mutex Logger::init_mutex;
