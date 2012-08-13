#include "logger.h"
#include <iostream>
#include <stdexcept>

// -------------------------
// Class LogStream
// -------------------------

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

void LogStream::setstate(std::ios_base::iostate state) { local_stream->setstate(state); }

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
Logger::Logger(size_t level, std::string filename, bool dual) :
	static_loglevel(level),
	local_stream(),
	loglevel(),
#else
Logger::Logger(size_t level, std::string filename, bool dual) :
	static_loglevel(level),
	local_stream(NULL),
	loglevel(NULL),
#endif // LOG_MULTI_THREAD
	logfilename(filename)
{
	logdual = dual;
	getDeafstream().setstate(std::ios_base::eofbit);
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
	*local_stream << "*** Started logging (" << LOGGER_VERSION << ") @" << getTime() << \
		"with log level: " << buffer[level] << " ***" << Logger::flush;
}

Logger::Logger(Logger&) : static_loglevel(DEFAULT_LOG_THRESHOLD) {}




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
	loglevel.reset();
#else
	delete loglevel;
#endif // LOG_MULTI_THREAD
	log_ptr.reset(static_cast<Logger*>(NULL));
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

std::string Logger::getLevelStr()
{
	return std::string(buffer[getLevel()]);
}

std::string Logger::getFilename()
{
	return logfilename;
}

LogStream& Logger::operator() (size_t level)
{
	if(willBeLogged(level))
		return resetStreamLevel(level);
	resetStream(NULL);
	return getDeafstream();
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

const char* const Logger::buffer[] = {
	"ERROR", "WARNING", "INFO",
	"DEBUG0", "DEBUG1", "DEBUG2",
	"DEBUG3" };

// Allocating and initializing Logger's static data member.  
// The smart pointer is allocated - not the object itself.
boost::shared_ptr<Logger> Logger::log_ptr;
std::ofstream* Logger::logfile(NULL);
bool Logger::logdual(false);

#ifdef LOG_MULTI_THREAD
boost::mutex Logger::init_mutex;
#endif // LOG_MULTI_THREAD
