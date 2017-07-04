#pragma once
#include <sstream>
#include <cassert>
#include <string>
#include <stdexcept>

#include <boost/thread/tss.hpp>
#include <boost/filesystem.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>

#include "colorlayout.h"
#include "logging_ctrl.h"

#define LOGGER_DEFAULT_LEVEL Logger::WARNING

namespace visionary_logger {
std::string print_backtrace() __attribute__ ((unused));
std::string make_syslog_prefix() __attribute__ ((unused));
/// Prints a message to syslog, a prefix with general information will be added
/// to the message
/// Note: Don't use it in signal handlers
void write_to_syslog(std::string msg) __attribute__((unused));
/// Prints a message to syslog, a prefix with general information will be added
/// to the message
/// Note: Don't use it in signal handlers
void write_to_syslog(
    std::string msg,
    log4cxx::LevelPtr level,
    log4cxx::LoggerPtr logger,
    log4cxx::spi::LocationInfo loc) __attribute__((unused));
}

/// logger macros that print an additional backtrace
#define LOG4CXX_DEBUG_BACKTRACE(logger, message) { \
	LOG4CXX_DEBUG(logger, message); \
	LOG4CXX_DEBUG(logger, visionary_logger::print_backtrace()); }

#define LOG4CXX_TRACE_BACKTRACE(logger, message) { \
	LOG4CXX_TRACE(logger, message); \
	LOG4CXX_TRACE(logger, visionary_logger::print_backtrace()); }

#define LOG4CXX_INFO_BACKTRACE(logger, message) { \
	LOG4CXX_INFO(logger, message); \
	LOG4CXX_INFO(logger, visionary_logger::print_backtrace()); }

#define LOG4CXX_WARN_BACKTRACE(logger, message) { \
	LOG4CXX_WARN(logger, message); \
	LOG4CXX_WARN(logger, visionary_logger::print_backtrace()); }

// same same
#define LOG4CXX_ERROR_BACKTRACE(logger, message) LOG4CXX_ERROR(logger, message)
#define LOG4CXX_FATAL_BACKTRACE(logger, message) LOG4CXX_FATAL(logger, message)

/// We redefine the log4cxx's ERROR marco to include a backtrace.
/// (copied macro definition from log4cxx/logger.h)
/// We always log to syslog at the cost of alwasy formating the message
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger, message)                                                             \
	{                                                                                              \
		log4cxx::LevelPtr level = ::log4cxx::Level::getError();                                    \
		::log4cxx::spi::LocationInfo location = LOG4CXX_LOCATION;                                  \
		::log4cxx::helpers::MessageBuffer oss_;                                                    \
		std::string throw_message(oss_.str(oss_ << message));                                      \
		if (logger->isErrorEnabled()) {                                                            \
			logger->forcedLog(                                                                     \
			    level, oss_.str(oss_ << visionary_logger::print_backtrace()), location);           \
		}                                                                                          \
		visionary_logger::write_to_syslog(throw_message, level, logger, location);                 \
	}

/// We redefine the log4cxx's FATAL marco to include a backtrace and to throw a
/// runtime_error (copied macro definition from log4cxx/logger.h)
/// We always log to syslog at the cost of alwasy formating the message
#undef LOG4CXX_FATAL
#define LOG4CXX_FATAL(logger, message)                                                             \
	{                                                                                              \
		log4cxx::LevelPtr level = ::log4cxx::Level::getFatal();                                    \
		::log4cxx::spi::LocationInfo location = LOG4CXX_LOCATION;                                  \
		::log4cxx::helpers::MessageBuffer oss_;                                                    \
		std::string throw_message(oss_.str(oss_ << message));                                      \
		if (logger->isFatalEnabled()) {                                                            \
			logger->forcedLog(                                                                     \
			    level, oss_.str(oss_ << visionary_logger::print_backtrace()), location);           \
		}                                                                                          \
		visionary_logger::write_to_syslog(throw_message, level, logger, location);                 \
		throw std::runtime_error(throw_message);                                                   \
	}

/// Get a log4cxx logger, while mimik the configuration behaviour of the old logger
/// The logger will only be configure, if neither the root logger nor the
/// Default logger has an appender
log4cxx::LoggerPtr get_default_logger(std::string logger_name, log4cxx::LevelPtr level, std::string fname, bool dual);

void configure_default_logger(log4cxx::LoggerPtr logger,
		log4cxx::LevelPtr level, std::string fname, bool dual);

/// gets the "Default" instance from log4cxx
inline
log4cxx::Logger&
get_log4cxx(
	// don't change default argument. It's an empty "smart" pointer on purpose
	log4cxx::LevelPtr level = log4cxx::LevelPtr(),
	std::string logger_name = "Default",
	std::string fname = std::string(),
	bool dual = false)
{
	static log4cxx::Logger* _logger;
	if (!_logger)
	{
		// never ever touch the allmighty &* ;)
		//   http://osdir.com/ml/apache.logging.log4cxx.devel/2004-11/msg00028.html
		_logger = &*get_default_logger(logger_name, level, fname, dual);
	}
	return *_logger;
}

struct Message
{
	std::ostringstream& get() { return _stream; }
	log4cxx::LevelPtr level() const { return _level; }

	Message(log4cxx::LevelPtr level) :
		_level(level), _stream() {}

	~Message()
	{
		// do the actual logging (triggered by reset of Logger's _buffer)
		get_log4cxx().log(level(), get().str(), LOG4CXX_LOCATION);
	}

	static void custom_cleanup(Message*)
	{
		// is only called on last delete of Message
		// don't do anything to workaround multi thread issues
		// FIXME
	}

private:
	log4cxx::LevelPtr _level;
	std::ostringstream _stream;
};


struct NullStream :
	public std::ostream
{
	struct nullbuf : public std::streambuf
	{
		int overflow(int c) { return traits_type::not_eof(c); }
	} m_sbuf;

	NullStream() : std::ios(&m_sbuf), std::ostream(&m_sbuf) {}
};


class Logger
{
private:
	boost::thread_specific_ptr<Message> _buffer;
	NullStream _null;
	size_t _level;
	size_t _last_level;

	Logger(size_t level = LOGGER_DEFAULT_LEVEL) :
		_buffer(&Message::custom_cleanup),
		_null(),
		_level(level),
		_last_level(level)
	{}

public:
	static
	log4cxx::LevelPtr log4cxx_level(size_t level)
	{
		switch(level)
		{
			case ERROR:
				return log4cxx::Level::getError();
			case WARNING:
				return log4cxx::Level::getWarn();
			case INFO:
				return log4cxx::Level::getInfo();
			case DEBUG0:
				return log4cxx::Level::getDebug();
			case DEBUG1:
				return log4cxx::Level::getTrace();
			default:
				return log4cxx::Level::getAll();
		}
	}

	enum levels {ERROR=0, WARNING=1, INFO=2, DEBUG0=3, DEBUG1=4, DEBUG2=5, DEBUG3=6};

	static
	levels logger_level(log4cxx::LevelPtr level)
	{
		if (level->isGreaterOrEqual(log4cxx::Level::getError()))
			return ERROR;
		if (level->isGreaterOrEqual(log4cxx::Level::getWarn()))
			return WARNING;
		if (level->isGreaterOrEqual(log4cxx::Level::getInfo()))
			return INFO;
		if (level->isGreaterOrEqual(log4cxx::Level::getDebug()))
			return DEBUG0;
		if (level->isGreaterOrEqual(log4cxx::Level::getTrace()))
			return DEBUG1;
		return DEBUG2;
	}


	static Logger& instance(
		std::string logger_name = "Default",
		size_t level = LOGGER_DEFAULT_LEVEL,
		std::string file = "",
		bool dual = false)
	{
		get_log4cxx(log4cxx_level(level), logger_name, file, dual);
		static Logger _logger(level);
		return _logger;
	}

	//! Returns threshold level of the Logger instance
	size_t getLevel()
	{
		return _level;
	}

	//! Returns whether given log level will produce output
	bool willBeLogged(size_t level)
	{
		return level <= getLevel();
	}

	//! Returns threshold level of the Logger instance
	std::string getLevelStr()
	{
		return log4cxx_level(getLevel())->toString();
	}

	//! Returns filename of the output file
	std::string getFilename() __attribute__((deprecated))
	{
		return "unsupported";
	}

	//! Get stream instance
	std::ostream&
	operator() (size_t level)
	{
		_last_level = level;
		if (_buffer.get() != NULL) {
			auto m = _buffer.release();
			delete m;
		}
		_buffer.reset(new Message(log4cxx_level(level)));
		return _buffer->get();
	}

	static std::ostream& flush(std::ostream& stream)
	{
		return std::flush(stream);
	}

	template <typename T>
	std::ostream&
	operator<<(T const& val)
	{
		if (willBeLogged(_last_level))
			return _buffer->get() << val;
		else
			return _null << val;
	}
};

class LoggerMixin
{
public:
	LoggerMixin() : mLog(Logger::instance()) {}
	LoggerMixin(LoggerMixin const&) : mLog(Logger::instance()) {}

	LoggerMixin& operator=(LoggerMixin const&) { return *this; }

protected:
	Logger& mLog;
};

