#pragma once
#include <fstream>
#include <iostream>
#include <string>

#include <chatty/chatty.h>

class Logger;
class Logger
{
private:
	typedef chatty::Level<> level_t;
	typedef chatty::Tee<
		std::ostream&,
		std::ofstream&
		> tee_t;
	typedef chatty::ThreadLocal<tee_t> drain_t;
	typedef chatty::Logger<drain_t&> logger_t;

	logger_t&    _ref;
	std::string  _fname;

	Logger(logger_t& ref, std::string fname) :
		_ref(ref), _fname(fname) {}

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
			size_t level = WARNING,
			std::string file = "",
			bool dual = false)
	{
		static std::ofstream f(file.empty() ? "/dev/null" : file.c_str());
		static std::ostream  o(NULL);

		static tee_t   t(chatty::tee(!(file.empty() || dual) ? o : std::cout, f));
		static drain_t d(t);
		static Logger  l(chatty::instance(d, level_t(level)), file);

		return l;
	}

	//! Returns threshold level of the Logger instance
	size_t getLevel()
	{
		return _ref.level().level();
	}

	//! Returns whether given log level will produce output
	bool willBeLogged(size_t level)
	{
		return level <= static_cast<size_t>(_ref.level().level());
	}

	//! Returns threshold level of the Logger instance
	std::string getLevelStr()
	{
		return std::string(_ref.level().c_str());
	}

	//! Returns filename of the output file
	std::string getFilename()
	{
		return _fname;
	}

	//! Get stream instance
	typename logger_t::mute_t&
	operator() (size_t level = DEBUG0)
	{
		return _ref(level_t(level));
	}

	static std::ostream& flush(std::ostream& stream)
	{
		return std::flush(stream);
	}

	template <typename T>
	typename logger_t::mute_t&
	operator<<(T const& val)
	{
		return _ref(level_t(INFO));
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
