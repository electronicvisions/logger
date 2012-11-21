#pragma once
#include <fstream>
#include <iostream>
#include <cassert>
#include <string>

#include <boost/scoped_ptr.hpp>
#include <log4cxx/logger.h>

struct Stream
{
	std::ostringstream& get() { return _stream; }
	size_t level() const { return _level; }

	Stream(size_t level = 0) :
		_level(level), _stream() {}

private:
	size_t _level;
	std::ostringstream _stream;
};

#define LOGGER_DEAULT_LEVEL WARNING

class Logger
{
private:
	static log4cxx::Logger& getLogger(size_t level = LOGGER_DEAULT_LEVEL)
	{
		static bool _initalized = false;
		static log4cxx::Logger* _logger = NULL;
		if (!_initalized) {
			// never ever touch the amazing &* ;)
			//   http://osdir.com/ml/apache.logging.log4cxx.devel/2004-11/msg00028.html
			_logger = &*log4cxx::Logger::getLogger("Default");
			_logger->setLevel(levelPtr(level));
		}
		return *_logger;
	}

	boost::scoped_ptr<Stream> _buffer;
	std::ofstream _null;

	Logger(size_t level = LOGGER_DEAULT_LEVEL) :
		_buffer(new Stream),
		_null("/dev/null")
	{}

	inline
	static log4cxx::LevelPtr levelPtr(int level)
	{
		using log4cxx::Level;
		switch (level) {
			case FATAL:
				return Level::getFatal();
			case ERROR:
				return Level::getError();
			case WARNING:
				return Level::getWarn();
			case INFO:
				return Level::getInfo();
			case DEBUG:
				return Level::getDebug();
			default:
				return Level::getTrace();
		}
	}

public:
	enum levels {
		FATAL   = 0,
		ERROR   = 1,
		WARNING = 2,
		INFO    = 3,
		DEBUG   = 4,
		DEBUG0  = DEBUG,
		DEBUG1  = DEBUG,
		DEBUG2  = DEBUG,
		DEBUG3  = DEBUG
	};

	static Logger& instance(
			size_t level = LOGGER_DEAULT_LEVEL,
			std::string file = "",
			bool dual = false)
	{
		Logger::getLogger(level);
		static Logger _logger(level);
		return _logger;
	}

	//! Returns threshold level of the Logger instance
	size_t getLevel()
	{
		assert(getLogger().getLevel() && "init of log4cxx::logger is broken");
		return getLogger().getLevel()->toInt();
	}

	//! Returns whether given log level will produce output
	bool willBeLogged(size_t level)
	{
		return level <= getLevel();
	}

	//! Returns threshold level of the Logger instance
	std::string getLevelStr()
	{
		return getLogger().getLevel()->toString();
	}

	//! Returns filename of the output file
	std::string getFilename() __attribute__((deprecated))
	{
		return "unsupported";
	}

	//! Get stream instance
	typename std::ostream&
	operator() (size_t level = DEBUG0)
	{
		if (willBeLogged(level)) {
			getLogger().forcedLog(
				//::log4cxx::Level(_buffer->level()),
				::log4cxx::Level::getInfo(),
				_buffer->get().str(),
				LOG4CXX_LOCATION);

			_buffer.reset(new Stream(level));

			return _buffer->get();
		}

		return _null;
	}

	static std::ostream& flush(std::ostream& stream)
	{
		return std::flush(stream);
	}

	template <typename T>
	typename std::ostream&
	operator<<(T const& val)
	{
		return this->operator() () << val;
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
