#include "logger.h"

// -------------------------
// Class LogStream
// -------------------------
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

LogStream::~LogStream() { delete local_stream; }

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
#ifdef MULTI_THREAD
Logger::Logger(size_t level, std::string filename, bool dual) : local_stream(&del_local_stream), logfilename(filename), loglevel(level)
#else
Logger::Logger(size_t level, std::string filename, bool dual) : local_stream(NULL), logfilename(filename), loglevel(level)
#endif // MULTI_THREAD
{
	deafstream.setstate(std::ios_base::eofbit);
	logdual = dual;
	resetStream();

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
	*local_stream << "*** Started logging @" << getTime() << " ***" << Logger::flush;
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
	strftime(buffer, sizeof(buffer), "%x %X", localtime_r(&t, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld ", buffer, (long)tv.tv_usec / 1000);
	return result;
}

#endif //WIN32

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

inline LogStream& Logger::formatStream(size_t level)
{
	*local_stream << getTime();
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	*local_stream << toColor(level);
#endif //WIN32
	local_stream->width(10);
	*local_stream << std::left << buffer[level];
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	*local_stream << resetColor() << ": ";
#endif //WIN32
	return *local_stream;
}

inline void Logger::resetStream(LogStream* stream)
{
#ifdef MULTI_THREAD
	local_stream.reset(stream);
#else
	if (local_stream != NULL)
		*local_stream << flush;
	delete local_stream;
	local_stream = stream;
#endif // MULTI_THREAD
}

inline LogStream& Logger::resetStream(size_t level)
{
	resetStream();
	return formatStream(level);
}

Logger::~Logger()
{
	resetStream((LogStream*)NULL);
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
}

Logger& Logger::instance(
		size_t level,             //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
		std::string filename,     //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
		bool dual
		)
{
#ifdef MULTI_THREAD
	boost::mutex::scoped_lock lock(init_mutex);
#endif // MULTI_THREAD
	if(!log_ptr)
	{
		if ( level > DEBUG3 ) level = DEBUG3;
		log_ptr = boost::shared_ptr<Logger> (new Logger(level, filename, dual));
	}
	return *log_ptr;
}

size_t Logger::getLevel()
{
	return loglevel;
}

std::string Logger::getLevelStr()
{
	return std::string(buffer[loglevel]);
}

std::string Logger::getFilename()
{
	return logfilename;
}

bool Logger::willBeLogged(size_t level)
{
	return (level <= loglevel);
}

LogStream& Logger::operator() (size_t level)
{
	if(willBeLogged(level))	return resetStream((size_t)level);
	return deafstream;
}

LogStream::LogStream& Logger::operator<<(stream_manip manip) {
	return *local_stream << manip;
}

LogStream::LogStream& Logger::operator<<(log_stream_manip manip) {
	return *local_stream << manip;
}

LogStream& Logger::flush(LogStream& stream)
{
	return stream.flush(stream);
}

const char* const Logger::buffer[] = {
	"ERROR", "WARNING", "INFO",
	"DEBUG0", "DEBUG1", "DEBUG2",
	"DEBUG3" };

// Allocating and initializing Logger's static data member.  
// The smart pointer is allocated - not the object itself.
boost::shared_ptr<Logger> Logger::log_ptr;
std::ofstream* Logger::logfile(NULL);
bool Logger::logdual(false);

#ifdef MULTI_THREAD
boost::mutex Logger::init_mutex;

inline void del_local_stream( LogStream* stream )
{
	stream->writeOut();
	delete stream;
	stream = NULL;
}
#endif // MULTI_THREAD
