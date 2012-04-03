// Singleton implementation of Logger class for symap2ic project
// by Sebastian Jeltsch, sjeltsch@kip.uni-heidelberg.de
// by Daniel Bruederle, bruederle@kip.uni-heidelberg.de
//
// IMMER LOGGER BLEIBEN!
//
// 2010-03-01


#ifndef __LOGGER_H__
#define __LOGGER_H__

#define LOG_MULTI_THREAD

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/shared_ptr.hpp>

#ifdef LOG_MULTI_THREAD
#define LOGGER_VERSION "MT"
#ifndef PYPLUSPLUS
	#include <boost/thread/thread.hpp>
	#include <boost/thread/mutex.hpp>
	#include <boost/thread/tss.hpp>
#endif
#else
#define LOGGER_VERSION "non-MT"
#endif // LOG_MULTI_THREAD

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
class LogStream;
class Logger;

using std::size_t;

//! Stream class used by the Logger
/*! Stream class used by the Logger. There is no need to use this class stand alone.
  If you are interested in the Logger itself you should skip this section and
  start your examination further down. */
class LogStream
{
	protected:
		std::ostringstream* local_stream;
		typedef std::ostream& (*stream_manip)(std::ostream&);
		typedef LogStream& (*log_stream_manip)(LogStream&);
	private:

		LogStream& getDeafstream();
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
			LogStream& operator<<(const T& val)
			{
				if (local_stream->bad()) return getDeafstream();
				*local_stream << val;
				return *this;
			}

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

		// emulate parts of std::ostringstream interface
		void setstate ( std::ios_base::iostate state );
		void clear();
		bool bad();
		bool eof();
		bool good();
		std::streamsize width () const;
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
	private:
		size_t const static_loglevel;
#ifdef LOG_MULTI_THREAD
#ifndef PYPLUSPLUS
		boost::thread_specific_ptr<LogStream> local_stream;
		static boost::mutex init_mutex;
		boost::thread_specific_ptr<size_t> loglevel;
#endif
#else
		LogStream* local_stream;
		size_t* loglevel;
#endif // LOG_MULTI_THREAD
		static boost::shared_ptr<Logger> log_ptr;

		static std::ofstream* logfile;
		std::string logfilename;
		static LogStream deafstream;
		static bool logdual;

		explicit Logger(size_t level, std::string filename, bool dual);
		Logger(Logger&);

		// Formatting the log messages
		std::string getTime();
#ifdef LOG_COLOR_OUTPUT
		const char* toColor(size_t level) const;
		const char* resetColor() const;
#endif // LOG_COLOR_OUTPUT
		LogStream& formatStream(size_t level);
		//! Contains the criticality tags for stream formatting
		static const char* const buffer[];

		//! Flush the old local stream and establish a new local string
		void resetStream(LogStream* stream);
		//! Flush the old local stream and establish a new formated local string with level tag
		LogStream& resetStreamLevel(size_t level);

	public:
		~Logger();

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

		//! Returns threshold level of the Logger instance
		std::string getLevelStr();

		//! Returns filename of the output file
		std::string getFilename();

		//! Tells if the passed logging level is sufficient to pass the logger's criticality threshold
		bool willBeLogged(size_t level);

		//! Get stream instance
		LogStream& operator() (size_t level=DEFAULT_LOG_LEVEL);

		//! Stream operator for data in multi-line comments
		template <typename T>
			LogStream& operator<<(const T& val)
			{
#ifndef PYPLUSPLUS
#ifdef LOG_MULTI_THREAD
				if (local_stream.get())
#else
				if (local_stream)
#endif // LOG_MULTI_THREAD
					return *local_stream << val;
#endif // !PYPLUSPLUS
				return deafstream;
			}

		//! Catches stream manupulators like std::ostream manipulators or LogStream stream manipulators in multi-line comments
		template <typename T>
			LogStream& operator<<(T& (*__fp)(T&))
			{
#ifndef PYPLUSPLUS
#ifdef LOG_MULTI_THREAD
				if(local_stream.get())
#else
				if (local_stream)
#endif // LOG_MULTI_THREAD
					return *local_stream << __fp;
#endif // !PYPLUSPLUS
				return deafstream;
			}

		//! Forced flush; ATTENTION afterwards the multi-line feature won't work anymore
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
		//! provide private interface for LogStream class
		friend class LogStream;
		friend class Logger::AlterLevel;
};


inline LogStream& LogStream::getDeafstream()
{
	return Logger::deafstream;
}

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

inline LogStream& LogStream::flush(LogStream& stream)
{
	stream.writeOut();
	stream.local_stream->setstate(std::ios_base::badbit);
	return stream;
}


inline size_t Logger::getLevel() {
#ifndef PYPLUSPLUS
#ifdef LOG_MULTI_THREAD
	if (!loglevel.get())
		loglevel.reset(new size_t(static_loglevel));
#else
#endif // LOG_MULTI_THREAD
	return *loglevel;
#endif // !PYPLUSPLUS
}

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
	strftime(buffer, sizeof(buffer), "%y-%m-%d %X", localtime_r(&t, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld ", buffer, (long)tv.tv_usec / 1000);
	return result;
}

#endif //WIN32

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

inline LogStream& Logger::formatStream(size_t level)
{
#ifndef PYPLUSPLUS
#ifdef LOG_COLOR_OUTPUT
	*local_stream << COLOR_RESET << getTime();
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	*local_stream << toColor(level);
#endif //WIN32
	local_stream->width(10);
	*local_stream << std::left << buffer[level];
#if not defined(WIN32) || not defined(_WIN32) || not defined(__WIN32__)
	*local_stream << resetColor() << ": ";
#endif //WIN32
#else // LOG_COLOR_OUTPUT
	*local_stream  << getTime();
	local_stream->width(10);
	*local_stream << std::left << buffer[level] << ": ";
#endif // LOG_COLOR_OUTPUT

	return *local_stream;
#endif // PYPLUSPLUS
}

inline void Logger::resetStream(LogStream* stream)
{
#ifdef LOG_MULTI_THREAD
#ifndef PYPLUSPLUS
	local_stream.reset(stream);
#endif
#else
	delete local_stream;
	local_stream = stream;
#endif // LOG_MULTI_THREAD
}

inline LogStream& Logger::resetStreamLevel(size_t level)
{
	resetStream(new LogStream);
	if (logfile) logfile->flush();
	return formatStream(level);
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
	protected:
		Logger& mLog;
};

#endif // __LOGGER_H__
