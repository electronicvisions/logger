// Singleton implementation of Logger class for symap2ic project
// by Daniel Bruederle, bruederle@kip.uni-heidelberg.de
// by Sebastian Jeltsch, sjeltsch@kip.uni-heidelberg.de
//
// IMMER LOGGER BLEIBEN!
//
// 2010-03-01


#ifndef __LOGGER_H__
#define __LOGGER_H__

#define MULTI_THREAD

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <boost/shared_ptr.hpp>

#ifdef MULTI_THREAD
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#endif // MULTI_THREAD

// Color definitions
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
#define COLOR_ERROR     COLOR_RED
#define COLOR_WARNING   COLOR_YELLOW
#define COLOR_DEFAULT   COLOR_GREEN

//! Singleton implementation of Logger class
/*! Only one single instance of this class can be created by calling the public function instance(). 
  Every further call returns a reference to this one class, arguments will be ignored, i.e. the 
  parameters loglevel and logfile can not be changed after their first initialization. 
  Pre-defined criticality levels are: ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3.
  The default criticality threshold of a logger is WARNING. If a message is streamed into the logger,
  it will only be recorded if the criticality of the message is at least as high as the threshold of
  the logger. In case you stream into a logger with no explicit criticality level, the default level of
  your message is DEBUG. */
class Logger
{
	private:
#ifdef MULTI_THREAD
		boost::thread_specific_ptr< std::ostringstream > local_stream;
		static boost::mutex init_mutex;
#else
		std::ostringstream* local_stream;
#endif // MULTI_THREAD
		static boost::shared_ptr<Logger> log_ptr;

		static std::ofstream* logfile;
		std::string logfilename;
		size_t loglevel;
		std::ostringstream deafstream;

		explicit Logger(size_t level, std::string filename);
		Logger(Logger&);

		// Formatting the log messages
		std::string getTime();
		std::string toString(size_t level);
		const char* toColor(size_t level) const;
		const char* resetColor() const;
		std::ostream& formatStream(size_t level);
		//! Contains the criticality tags for stream formatting
		static const char* const buffer[];

		//! Returns std::cout or ofstream reference depending on the chosen output method
		static std::ostream& getOutStream();

		//! typedef for pointer to function that takes an ostream and returns an ostream
		typedef std::ostream& (*stream_manip)(std::ostream&);

#ifdef MULTI_THREAD
		//! Delegate destructor for local steams
		friend void del_local_stream( std::ostringstream* );
#endif // MULTI_THREAD

	public:
		/*! The criticality levels of this logger class. 
		  Messages streamed into the logger will only be recorded if their criticality is at least the 
		  criticality level of the logger. */
#ifdef DEBUG
#undef DEBUG
		enum levels {ERROR=0, WARNING=1, INFO=2, DEBUG=3, DEBUG1=4, DEBUG2=5, DEBUG3=6};
#define DEBUG
#else
		enum levels {ERROR=0, WARNING=1, INFO=2, DEBUG=3, DEBUG1=4, DEBUG2=5, DEBUG3=6};
#endif

		~Logger();

		/*! This is the only way to create an instance of a Logger. Only the first call actually creates an instance, 
		  all further calls return a reference to the one and only instance. */
		static Logger& instance(
				size_t level=WARNING,       //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
				std::string filename=""     //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
				);

		//! Returns threshold level of the Logger instance
		size_t getLevel();

		//! Returns filename of the output file
		std::string getFilename();

		//! Tells if the passed logging level is sufficient to pass the logger's criticality threshold
		bool willBeLogged(size_t level);

		//! Get stream instance
		std::ostream& operator() (size_t level=DEBUG);

		template <typename T>
			std::ostream& operator<<(const T& val)
			{
				return *local_stream << val;
			}

		//! Get stream instance for multi-line comments via stream operator
		std::ostream& operator<<(stream_manip manip) {
			return manip((*this)());
		}
};

#ifdef MULTI_THREAD
//! Delegate destructor for local steams
extern void del_local_stream( std::ostringstream* stream );
#endif // MULTI_THREAD

#endif // __LOGGER_H__
