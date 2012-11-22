#pragma once
#include <fstream>
#include <iostream>
#include <cassert>
#include <string>

#include <boost/thread/tss.hpp>
#include <log4cxx/logger.h>


#define LOGGER_DEAULT_LEVEL Logger::WARNING

/// gets the "Default" instance from log4cxx
log4cxx::Logger&
get_log4cxx(log4cxx::LevelPtr level = log4cxx::Level::getWarn());

struct Message
{
	std::ostringstream& get() { return _stream; }
	log4cxx::LevelPtr level() const { return _level; }

	Message(log4cxx::LevelPtr level) :
		_level(level), _stream() {}

	~Message()
	{
		// do the actual logging
		get_log4cxx().forcedLog(level(), get().str(), LOG4CXX_LOCATION);
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

	Logger(size_t level = LOGGER_DEAULT_LEVEL) :
		_buffer(),
		_null()
	{
		_buffer.reset(new Message(log4cxx::Level::toLevel(level)));
	}

public:
	enum levels {
		FATAL   = log4cxx::Level::FATAL_INT,
		ERROR   = log4cxx::Level::ERROR_INT,
		WARNING = log4cxx::Level::WARN_INT,
		INFO    = log4cxx::Level::INFO_INT,
		DEBUG   = log4cxx::Level::DEBUG_INT,
		DEBUG0  = DEBUG,
		DEBUG1  = log4cxx::Level::TRACE_INT,
		DEBUG2  = log4cxx::Level::ALL_INT,
		DEBUG3  = DEBUG2
	};

	static Logger& instance(
			size_t level = LOGGER_DEAULT_LEVEL,
			std::string file = "",
			bool dual = false)
	{
		get_log4cxx(log4cxx::Level::toLevel(level));
		static Logger _logger(level);
		return _logger;
	}

	//! Returns threshold level of the Logger instance
	size_t getLevel()
	{
		assert(get_log4cxx().getLevel() && "init of log4cxx::logger is broken");
		return get_log4cxx().getLevel()->toInt();
	}

	//! Returns whether given log level will produce output
	bool willBeLogged(size_t level)
	{
		return level >= getLevel();
	}

	//! Returns threshold level of the Logger instance
	std::string getLevelStr()
	{
		assert(get_log4cxx().getLevel() && "init of log4cxx::logger is broken");
		return get_log4cxx().getLevel()->toString();
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
		if (willBeLogged(level))
		{
			_buffer.reset(new Message(log4cxx::Level::toLevel(level)));
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


inline
log4cxx::Logger&
get_log4cxx(log4cxx::LevelPtr level)
{
	static bool _initalized = false;
	static log4cxx::Logger* _logger = NULL;
	if (!_initalized) {
		// never ever touch the allmighty &* ;)
		//   http://osdir.com/ml/apache.logging.log4cxx.devel/2004-11/msg00028.html
		_logger = &*log4cxx::Logger::getLogger("Default");
		_logger->setLevel(level);
	}
	return *_logger;
}
