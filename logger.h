#pragma once
#include <sstream>
#include <cassert>
#include <string>
#include <stdexcept>

#include <boost/thread/tss.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>

#include "log4cxx/colorlayout.h"


#define LOGGER_DEAULT_LEVEL Logger::WARNING

/// gets the "Default" instance from log4cxx
log4cxx::Logger&
get_log4cxx(
	// don't change default argument. It's an empty "smart" pointer on purpose
	log4cxx::LevelPtr level = log4cxx::LevelPtr(),
	std::string fname = std::string(),
	bool dual = false);

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

	Logger(size_t level = LOGGER_DEAULT_LEVEL) :
		_buffer(),
		_null(),
		_level(level)
	{}

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

public:
	enum levels {ERROR=0, WARNING=1, INFO=2, DEBUG0=3, DEBUG1=4, DEBUG2=5, DEBUG3=6};

	static Logger& instance(
			size_t level = LOGGER_DEAULT_LEVEL,
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
	typename std::ostream&
	operator() (size_t level = DEBUG0)
	{
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
get_log4cxx(log4cxx::LevelPtr level,
	std::string fname, bool dual)
{
	static log4cxx::Logger* _logger;
	if (!_logger)
	{
		if (fname.empty() && dual)
			throw std::logic_error("dual log mode requires a filename");

		if (fname.empty() || dual)
		{
			log4cxx::ConsoleAppender* console = new log4cxx::ConsoleAppender(
				log4cxx::LayoutPtr(new log4cxx::ColorLayout()));
			log4cxx::BasicConfigurator::configure(log4cxx::AppenderPtr(console));
		}

		if (!fname.empty())
		{
			log4cxx::FileAppender* file = new log4cxx::FileAppender(
				log4cxx::LayoutPtr(new log4cxx::ColorLayout(false)), "logfile", false);
			log4cxx::BasicConfigurator::configure(log4cxx::AppenderPtr(file));
		}

		// never ever touch the allmighty &* ;)
		//   http://osdir.com/ml/apache.logging.log4cxx.devel/2004-11/msg00028.html
		_logger = &*log4cxx::Logger::getLogger("Default");
		_logger->setLevel(level);
	}
	return *_logger;
}
