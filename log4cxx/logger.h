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

/// Get a log4cxx logger, while mimik the configuration behaviour of the old logger
/// The logger will only be configure, if neither the root logger nor the
/// Default logger has an appender
log4cxx::LoggerPtr get_default_logger(log4cxx::LevelPtr level, std::string fname, bool dual);

void configure_default_logger(log4cxx::LoggerPtr logger, log4cxx::LevelPtr level, std::string fname, bool dual);

/// gets the "Default" instance from log4cxx
inline
log4cxx::Logger&
get_log4cxx(
	// don't change default argument. It's an empty "smart" pointer on purpose
	log4cxx::LevelPtr level = log4cxx::LevelPtr(),
	std::string fname = std::string(),
	bool dual = false)
{
	static log4cxx::Logger* _logger;
	if (!_logger)
	{
		// never ever touch the allmighty &* ;)
		//   http://osdir.com/ml/apache.logging.log4cxx.devel/2004-11/msg00028.html
		_logger = &*get_default_logger(level, fname, dual);
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
		// do the actual logging
		get_log4cxx().log(level(), get().str(), LOG4CXX_LOCATION);
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
		_buffer(),
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
			size_t level = LOGGER_DEFAULT_LEVEL,
			std::string file = "",
			bool dual = false)
	{
		get_log4cxx(log4cxx_level(level), file, dual);
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

		if (willBeLogged(level))
		{
			_buffer.reset(new Message(log4cxx_level(level)));
			return _buffer->get();
		}

		return _null;
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

