#pragma once
// Singleton style Logger implementation
// by Sebastian Jeltsch, sjeltsch@kip.uni-heidelberg.de
// by Daniel Bruederle, bruederle@kip.uni-heidelberg.de
// frequent fixing by Eric Müller, mueller@kip.uni-heidelberg.de
//
// IMMER LOGGER BLEIBEN!
// Schoen waer's... langsam werd ich unlogger!!!
//
// 2010-03-01


#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/date_time.hpp>

#ifndef PYPLUSPLUS
	#include <boost/thread/thread.hpp>
	#include <boost/thread/mutex.hpp>
	#include <boost/thread/tss.hpp>
#endif

// Color definitions
#ifdef LOG_COLOR_OUTPUT
#define COLOR_BLACK     "\33[30m"
#define COLOR_RED       "\33[31m"
#define COLOR_GREEN     "\33[32m"
#define COLOR_YELLOW    "\33[33m"
#define COLOR_BLUE      "\33[34m"
#define COLOR_PURPLE    "\33[35m"
#define COLOR_MARINE    "\33[36m"
#define COLOR_GRAY      "\33[37m"
#define COLOR_RESET     "\33[0m"

// Formatting definitions
#define COLOR_ERROR              COLOR_RED
#define COLOR_WARNING            COLOR_YELLOW
#define COLOR_DEFAULT            COLOR_GREEN
#endif // LOG_COLOR_OUTPUT

// behaviour
#define DEFAULT_LOG_THRESHOLD    Logger::WARNING
#define DEFAULT_LOG_LEVEL        Logger::DEBUG0

// forward declarations
class Logger;

using std::size_t;

//! Stream class used by the Logger
/*! Stream class used by the Logger. There is no need to use this class stand alone.
  If you are interested in the Logger itself you should skip this section and
  start your examination further down. */
class LogStream
{
protected:
	std::ostringstream * local_stream;
	typedef std::ostream& (*stream_manip)(std::ostream&);
	typedef LogStream& (*log_stream_manip)(LogStream&);
private:
	//! Returns std::cout or filestream depending on first Logger instantiation
	std::ostream& getOutStream();
	//! Write local stream to output (file/terminal)
	void writeOut();
public:
	LogStream();
	virtual ~LogStream();

	//! handles LogStream& stream-ins
	LogStream& operator<<(LogStream& val);

	//! Forwards std::ostream& data to the member local_stream
	template <typename T>
	LogStream& operator<<(const T& val);

	//! Catches std::ostream format stream manipulators and forward to local stream
	LogStream& operator<<(stream_manip manip);

	//! Catches LogStream manipulators
	LogStream& operator<<(log_stream_manip manip);

	//! Defines the custom flush for the Logger
	static LogStream& flush(LogStream& stream);

	//! color modifier
#ifdef LOG_COLOR_OUTPUT
	static LogStream& black(LogStream& stream);
	static LogStream& red(LogStream& stream);
	static LogStream& green(LogStream& stream);
	static LogStream& yellow(LogStream& stream);
	static LogStream& blue(LogStream& stream);
	static LogStream& purple(LogStream& stream);
	static LogStream& marine(LogStream& stream);
	static LogStream& reset(LogStream& stream);
#endif // LOG_COLOR_OUTPUT

	void setstate(std::ios_base::iostate state);
	std::streamsize width (std::streamsize wide);
	std::string str();
};



//! Singleton implementation of Logger class
/*! Only one single instance of this class can be created by calling the public function instance().
  Every further call returns a reference to this one class, arguments will be ignored, i.e. the
  parameters loglevel and logfile can not be changed after their first initialization.
  Pre-defined criticality levels are: ERROR, WARNING, INFO, DEBUG0, DEBUG1, DEBUG2, DEBUG3.
  The default criticality threshold of a logger is WARNING. If a message is streamed into the logger,
  it will only be recorded if the criticality of the message is at least as high as the threshold of
  the logger. In case you stream into a logger with no explicit criticality level, the default level of
  your message is DEBUG0. */
class Logger
{
public:
	/*! The criticality levels of this logger class.
	  Messages streamed into the logger will only be recorded if their criticality is at least the
	  criticality level of the logger. */
	enum levels {ERROR=0, WARNING=1, INFO=2, DEBUG0=3, DEBUG1=4, DEBUG2=5, DEBUG3=6};

	/*! This is the only way to create an instance of a Logger. Only the first call actually creates an instance,
	  all further calls return a reference to the one and only instance. */
	static Logger& instance(
			size_t level=DEFAULT_LOG_THRESHOLD,  //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
			std::string filename="",             //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
			bool dual=false
		);

	//! Returns threshold level of the Logger instance
	size_t getLevel();

	//! Returns whether given log level will produce output
	bool willBeLogged(size_t level);

	//! Returns threshold level of the Logger instance
	std::string getLevelStr();

	//! Returns filename of the output file
	std::string getFilename();

	//! Get stream instance
	LogStream& operator() (size_t level=DEFAULT_LOG_LEVEL);

	//! Stream operator for data in multi-line comments
	template <typename T>
	LogStream& operator<<(const T& val);

	//! Catches stream manupulators like std::ostream manipulators or LogStream stream manipulators in multi-line comments
	template <typename T>
	LogStream& operator<<(T& (*__fp)(T&));

	//! Forced flush; afterwards the multi-line feature won't work anymore
	static LogStream& flush(LogStream& stream);

	// allows for log level escalation (RAII-style)
	class AlterLevel
	{
		public:
			explicit AlterLevel(size_t level);
			~AlterLevel();
		private:
			size_t old_level;
			// prevent heap allocation
			void* operator new(size_t size) throw(std::bad_alloc);
			void* operator new[](size_t size) throw(std::bad_alloc);
			void  operator delete(void *p);
			void  operator delete[](void *p);
	};

private:
	size_t const static_loglevel;

#ifndef PYPLUSPLUS
	boost::thread_specific_ptr<LogStream> local_logstream;
	static boost::mutex& getInit_mutex();
	boost::thread_specific_ptr<size_t> loglevel;
#endif // PYPLUSPLUS

	static std::ofstream& getLogfile();

	std::string logfilename;

	static bool& getLogdual();

	static LogStream& getDeafstream();

	explicit Logger(size_t level, std::string filename, bool dual);
	Logger(Logger const&);
	~Logger();

#ifdef LOG_COLOR_OUTPUT
	const char* toColor(size_t level) const;
	const char* resetColor() const;
#endif // LOG_COLOR_OUTPUT

	LogStream& formatStream(size_t level);
	//! Contains the criticality tags for stream formatting
	static char const * const * getBuffer();

	//! Flush the old local stream and establish a new local string
	void resetStream(LogStream* stream);
	//! Flush the old local stream and establish a new formated local string with level tag
	LogStream& resetStreamLevel(size_t level);

	//! provide private interface for LogStream class
	friend class LogStream;
	friend class Logger::AlterLevel;

	template<typename T>
	friend void boost::checked_delete(T*);
};


template <typename T>
LogStream& LogStream::operator<<(const T& val)
{
	if (local_stream->bad())
		return Logger::getDeafstream();
	(*local_stream) << val;
	return *this;
}

#ifndef PYPLUSPLUS // due to ifdefing of the declaration
inline boost::mutex& Logger::getInit_mutex() {
	static boost::mutex* init_mutex = NULL;
	if (!init_mutex) init_mutex = new boost::mutex;
	return *init_mutex;
}
#endif

inline std::ofstream& Logger::getLogfile() {
	static std::ofstream * _logfile = NULL;
	if (!_logfile)
		_logfile = new std::ofstream;
	return *_logfile;
}

inline bool& Logger::getLogdual() {
	static bool * _logdual = NULL;
	if (!_logdual) {
		_logdual = new bool;
		*_logdual = false;
	}
	return *_logdual;
}

inline char const * const * Logger::getBuffer() {
	static char const * const buffer[] = {
		"ERROR", "WARNING", "INFO",
		"DEBUG0", "DEBUG1", "DEBUG2",
		"DEBUG3"
	};
	return buffer;
}

inline LogStream& Logger::getDeafstream()
{
	static LogStream* _deafstream = NULL;
	if (!_deafstream)
		_deafstream = new LogStream;
	return *_deafstream;
}

inline std::ostream& LogStream::getOutStream()
{
	if (Logger::getLogfile().is_open()) return Logger::getLogfile();
	return std::cout;
}

inline void LogStream::writeOut()
{
	if (!local_stream->bad())
	{
		*local_stream << std::endl;
		getOutStream() << local_stream->str();
		if (Logger::getLogdual())
			std::cout << local_stream->str();
	}
}

inline LogStream& LogStream::flush(LogStream& stream)
{
	stream.writeOut();
	stream.local_stream->setstate(std::ios_base::badbit);
	return stream;
}

inline LogStream& Logger::formatStream(size_t level)
{
	using boost::posix_time::microsec_clock;
#ifndef PYPLUSPLUS
#ifdef LOG_COLOR_OUTPUT
	*local_stream << COLOR_RESET;
	*local_stream << microsec_clock::local_time() << " ";
	*local_stream << toColor(level);
	local_stream->width(10);
	*local_stream << std::left << buffer[level];
	*local_stream << resetColor() << ": ";
#else // LOG_COLOR_OUTPUT
	*local_logstream  << microsec_clock::local_time() << " ";
	local_logstream->width(10);
	*local_logstream << std::left << getBuffer()[level] << ": ";
#endif // LOG_COLOR_OUTPUT

	return *local_logstream;
#endif // PYPLUSPLUS
}

inline void Logger::resetStream(LogStream* stream)
{
#ifndef PYPLUSPLUS
	local_logstream.reset(stream);
#endif
}

inline LogStream& Logger::resetStreamLevel(size_t level)
{
	resetStream(new LogStream);
	return formatStream(level);
}


inline size_t Logger::getLevel() {
#ifndef PYPLUSPLUS
	if (!loglevel.get())
		loglevel.reset(new size_t(static_loglevel));
	return *loglevel;
#endif // !PYPLUSPLUS
}


template <typename T>
LogStream& Logger::operator<<(const T& val)
{
#ifndef PYPLUSPLUS
	if (local_logstream.get())
		return *local_logstream << val;
#endif // !PYPLUSPLUS
	return getDeafstream();
}

//! Catches stream manupulators like std::ostream manipulators or LogStream stream manipulators in multi-line comments
template <typename T>
LogStream& Logger::operator<<(T& (*__fp)(T&))
{
#ifndef PYPLUSPLUS
	if(local_logstream.get())
		return *local_logstream << __fp;
#endif // !PYPLUSPLUS
	return getDeafstream();
}

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


inline bool Logger::willBeLogged(size_t level)
{
	return level <= getLevel();
}




// Basic logger class that features a "prefix" in front of every message
class PrefixedLogger /* : public LoggerInterface */ {
	// we should refactor class Logger to implement a common "LoggerInterface"
	// and use the same interface here
	std::string prefix;
	Logger * log_ptr;

	PrefixedLogger() {}
	PrefixedLogger(std::string prefix, Logger * log_ptr) :
		prefix(prefix),
		log_ptr(log_ptr)
	{}

public:
	// factory method to emulate Logger's behavior
	static PrefixedLogger& instance(
		std::string prefix,
		size_t level=DEFAULT_LOG_THRESHOLD,
		std::string filename="",
		bool dual=false)
	{
		Logger * log_ptr = &(Logger::instance(level, filename, dual));
		return *(new PrefixedLogger(prefix, log_ptr));
	}

	PrefixedLogger& operator() (size_t level=DEFAULT_LOG_LEVEL) {
		(*log_ptr)(level);
		return *this;
	}

	template <typename T>
	LogStream& operator<<(const T& val)
	{
		return ((*log_ptr) << "[" << prefix << "] " << val);
	}

	template <typename T>
	LogStream& operator<<(T& (*__fp)(T&)) {
		return ((*log_ptr) << __fp);
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

#endif // __LOGGER_H__
