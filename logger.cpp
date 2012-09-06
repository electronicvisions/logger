#include "logger.h"
#include <iostream>
#include <stdexcept>

// -------------------------
// Class LogStream
// -------------------------

LogStream::LogStream() : local_stream() {}

LogStream::~LogStream()
{
	writeOut();
}

LogStream& LogStream::operator<<(LogStream& val)
{
	local_stream << val.str();
	return *this;
}

LogStream& LogStream::operator<<(stream_manip manip)
{
	manip(local_stream);
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

std::streamsize LogStream::width (std::streamsize wide)
{
	local_stream.width(wide);
	return wide;
}

void LogStream::setstate(std::ios_base::iostate state)
{
	local_stream.setstate(state);
}

std::string LogStream::str()
{
	return local_stream.str();
}


// -------------------------
// Class Logger
// -------------------------
Logger::Logger(size_t level, std::string filename, bool dual) :
	static_loglevel(level),
	local_stream(),
	loglevel(),
	logfilename(filename)
{
	logdual = dual;
	getDeafstream().setstate(std::ios_base::eofbit);
	resetStream(new LogStream);
	loglevel.reset(new size_t(level));

	if (!logfilename.empty())
		logfile.open(logfilename.c_str(), std::fstream::out | std::fstream::binary);

	if (logfilename.empty() && dual)
		throw std::runtime_error("dual logging requires file name");

	*local_stream << "*** Started logging @";
	*local_stream << boost::posix_time::second_clock::local_time();
	*local_stream << "with log level: " << buffer[level];
	*local_stream << " ***" << Logger::flush;
}

// hidden copy constructor
Logger::Logger(Logger const&) : static_loglevel(DEFAULT_LOG_THRESHOLD) {}




Logger::~Logger()
{
	resetStream(NULL);
	if (logfile.is_open()) {
		logfile.flush();
		logfile.close();
	}
	loglevel.reset();
}

Logger& Logger::instance(
		size_t level,             //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
		std::string filename,     //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
		bool dual
		)
{
	boost::mutex::scoped_lock lock(init_mutex);

	if(!_instance)
	{
		if ( level > DEBUG3 ) level = DEBUG3;
		_instance.reset(new Logger(level, filename, dual));
	}
	return *_instance;
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
boost::scoped_ptr<Logger> Logger::_instance;
std::ofstream Logger::logfile;
bool Logger::logdual(false);
boost::mutex Logger::init_mutex;
