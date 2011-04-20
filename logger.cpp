#include "logger.h"
#include <iostream>
#include <stdexcept>

// -------------------------
// Class LogStream
// -------------------------
inline LogStream& LogStream::getDeafstream()
{
	return Logger::deafstream;
}
inline std::ostream& LogStream::getOutStream()
{
	if (Logger::logfile) return *(Logger::logfile);
	return std::cout;
}

inline void LogStream::writeOut()
{
	if (!local_stream->bad())
	{
		*local_stream << std::endl;
		getOutStream() << local_stream->str();
		if (Logger::logdual)
			std::cout << local_stream->str();
	}
}

LogStream::LogStream() : local_stream(new std::ostringstream) {}

LogStream::~LogStream()
{
	this->writeOut();
	delete local_stream;
	local_stream=NULL;
}

LogStream& LogStream::operator<<(LogStream& val)
{
	*local_stream << val.str();
	return *this;
}

LogStream& LogStream::operator<<(stream_manip manip)
{
	manip(*local_stream);
	return *this;
}
LogStream& LogStream::operator<<(log_stream_manip manip)
{
	return manip(*this);
}

inline LogStream& LogStream::flush(LogStream& stream)
{
	stream.writeOut();
	stream.local_stream->setstate(std::ios_base::badbit);
	return stream;
}

#ifdef LOG_COLOR_OUTPUT
LogStream& LogStream::black(LogStream& stream)
{
	return stream << COLOR_BLACK;
}

LogStream& LogStream::red(LogStream& stream)
{
	return stream << COLOR_RED;
}

LogStream& LogStream::green(LogStream& stream)
{
	return stream << COLOR_GREEN;
}

LogStream& LogStream::yellow(LogStream& stream)
{
	return stream << COLOR_YELLOW;
}

LogStream& LogStream::blue(LogStream& stream)
{
	return stream << COLOR_BLUE;
}

LogStream& LogStream::purple(LogStream& stream)
{
	return stream << COLOR_PURPLE;
}

LogStream& LogStream::marine(LogStream& stream)
{
	return stream << COLOR_MARINE;
}

LogStream& LogStream::reset(LogStream& stream)
{
	return stream << COLOR_RESET;
}
#endif // LOG_COLOR_OUTPUT

void LogStream::setstate ( std::ios_base::iostate state ) { local_stream->setstate(state); }

void LogStream::clear() { local_stream->clear(); }

bool LogStream::bad() { return local_stream->bad(); }

bool LogStream::eof() { return local_stream->eof(); }

bool LogStream::good() { return local_stream->good(); }

std::streamsize LogStream::width () const
{
	return local_stream->width();
}

std::streamsize LogStream::width (std::streamsize wide)
{
	local_stream->width(wide);
	return wide;
}

std::string LogStream::str() { return local_stream->str(); }


// -------------------------
// Class Logger
// -------------------------
#ifdef LOG_MULTI_THREAD
Logger::Logger(size_t level, std::string filename, bool dual) : local_stream(), loglevel(), logfilename(filename)
#else
Logger::Logger(size_t level, std::string filename, bool dual) : local_stream(NULL), loglevel(NULL), logfilename(filename)
#endif // LOG_MULTI_THREAD
{
	deafstream.setstate(std::ios_base::eofbit);
	logdual = dual;
	resetStream(new LogStream);
#ifdef LOG_MULTI_THREAD
	loglevel.reset(new size_t(level));
#else
	loglevel = new size_t(level);
#endif // LOG_MULTI_THREAD

	if (logfilename != "")
	{
		logfile = new std::ofstream;
		logfile->open(logfilename.c_str(), (std::fstream::out | std::fstream::binary) );
		if (!logfile->is_open())
			throw std::runtime_error("Logger::ERROR: unable to open given logfile");
	}
	else
	{
		if (dual)
			throw std::runtime_error("Logger::ERROR: to use dual logging mode you need to provide a filename");
	}
	*local_stream << "*** Started logging @" << getTime() << "with log level: " << buffer[level] << " ***" << Logger::flush;
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
	char buffer[40];
	time_t t;
	time(&t);
	tm r;
	strftime(buffer, sizeof(buffer), "%y-%m-%d %X", localtime_r(&t, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld ", buffer, (long)tv.tv_usec / 1000);
	return result;
}

#endif //WIN32

#ifdef LOG_COLOR_OUTPUT
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
#endif // LOG_COLOR_OUTPUT

inline LogStream& Logger::formatStream(size_t level)
{
#ifdef LOG_COLOR_OUTPUT
	*local_stream << COLOR_RESET << getTime();
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	*local_stream << toColor(level);
#endif //WIN32
	local_stream->width(10);
	*local_stream << std::left << buffer[level];
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	*local_stream << resetColor() << ": ";
#endif //WIN32
#else // LOG_COLOR_OUTPUT
	*local_stream  << getTime();
	local_stream->width(10);
	*local_stream << std::left << buffer[level] << ": ";
#endif // LOG_COLOR_OUTPUT

	return *local_stream;
}

inline void Logger::resetStream(LogStream* stream)
{
#ifdef LOG_MULTI_THREAD
	local_stream.reset(stream);
#else
	delete local_stream;
	local_stream = stream;
#endif // LOG_MULTI_THREAD
}

inline LogStream& Logger::resetStreamLevel(size_t level)
{
	resetStream(new LogStream);
	return formatStream(level);
}

Logger::~Logger()
{
	resetStream(NULL);
	if(logfile)
	{
		if(logfile->is_open())
		{
			logfile->flush();
			logfile->close();
		}
		delete logfile;
		logfile = 0;
	}
#ifdef LOG_MULTI_THREAD
	loglevel.reset(NULL);
#else
	delete loglevel;
#endif // LOG_MULTI_THREAD
}

Logger& Logger::instance(
		size_t level,             //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
		std::string filename,     //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
		bool dual
		)
{
#ifdef LOG_MULTI_THREAD
	boost::mutex::scoped_lock lock(init_mutex);
#endif // LOG_MULTI_THREAD
	if(!log_ptr)
	{
		if ( level > DEBUG3 ) level = DEBUG3;
		log_ptr = boost::shared_ptr<Logger> (new Logger(level, filename, dual));
	}
	return *log_ptr;
}

size_t Logger::getLevel()
{
	return *loglevel;
}

std::string Logger::getLevelStr()
{
	return std::string(buffer[*loglevel]);
}

std::string Logger::getFilename()
{
	return logfilename;
}

bool Logger::willBeLogged(size_t level)
{
	return (level <= *loglevel);
}

LogStream& Logger::operator() (size_t level)
{
	if(willBeLogged(level))	return resetStreamLevel(level);
	resetStream(NULL);
	return deafstream;
}

LogStream& Logger::flush(LogStream& stream)
{
	return stream.flush(stream);
}

Logger::AlterLevel::AlterLevel(size_t level) {
	Logger& log = Logger::instance();
	old_level = *log.loglevel;
	*log.loglevel = level;
}

Logger::AlterLevel::~AlterLevel()
{
	Logger& log = Logger::instance();
	*log.loglevel = old_level;
}

void* Logger::AlterLevel::operator new(size_t size) throw(std::bad_alloc)
{
	throw std::bad_alloc();
	return NULL;
}

void  Logger::AlterLevel::operator delete(void *p) {}


const char* const Logger::buffer[] = {
	"ERROR", "WARNING", "INFO",
	"DEBUG0", "DEBUG1", "DEBUG2",
	"DEBUG3" };

// Allocating and initializing Logger's static data member.  
// The smart pointer is allocated - not the object itself.
boost::shared_ptr<Logger> Logger::log_ptr;
std::ofstream* Logger::logfile(NULL);
bool Logger::logdual(false);
LogStream Logger::deafstream;

#ifdef LOG_MULTI_THREAD
boost::mutex Logger::init_mutex;
#endif // LOG_MULTI_THREAD
