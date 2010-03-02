// Singleton implementation of Debugger class for symap2ic project
// by Daniel Bruederle, bruederle@kip.uni-heidelberg.de
// by Sebastian Jeltsch, sjeltsch@kip.uni-heidelberg.de
//
// IMMER LOGGER BLEIBEN!
//
// 2010-03-01


#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstddef>
#include <boost/shared_ptr.hpp>

//! Singleton implementation of Debugger class
/*! Only one single instance of this class can be created by calling the public function instance(). 
  Every further call returns a reference to this one class. The parameters loglevel and logfile can 
  not be changed after their first initialization. 
  Pre-defined logging levels are: ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3.
  Default log level ist DEBUG if you reference the the debugger wit no explicit log level */
class Logger
{
	private:
		size_t loglevel;
		static boost::shared_ptr<Logger> log_ptr;
		std::string logfilename;
		std::fstream* logfile;
		std::ostringstream deafStream;

		Logger(size_t level, std::string filename);
		Logger(Logger&);

		std::string getTime();
		std::string toString(size_t level);
		std::string toColor(size_t level);
		const std::string resetColor() const;

		std::ostream& getFileStream(size_t level);
		std::ostream& getStdStream(size_t level);

	public:
		enum levels {ERROR=0, WARNING=1, INFO=2, DEBUG=3, DEBUG1=4, DEBUG2=5, DEBUG3=6};

		~Logger();

		/*! This is the only way to create an instance of a Logger. Only the first call actually creates an instance, 
		  all further calls returns a reference to the one and only instance. */
		static Logger& instance(
				size_t level=WARNING,       //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
				std::string filename=""     //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
				);

		size_t getLevel();
		std::string getFilename();

		//! tells if the passed logging level is sufficient to pass the the logger's criticality threshold
		bool willBeLogged(unsigned int level);

		std::ostream& operator() (size_t level=DEBUG);

		std::ostream& operator<<(std::basic_string<char> val);
		std::ostream& operator<<(std::ostringstream& val);
		std::ostream& operator<<(long val);
		std::ostream& operator<<(unsigned long val);
		std::ostream& operator<<(bool val);
		std::ostream& operator<<(short val);
		std::ostream& operator<<(unsigned short val);
		std::ostream& operator<<(int val);
		std::ostream& operator<<(unsigned int val);
		std::ostream& operator<<(long long val);
		std::ostream& operator<<(unsigned long long val);
		std::ostream& operator<<(double val);
		std::ostream& operator<<(float val);
		std::ostream& operator<<(long double val);
		std::ostream& operator<<(const char* val);
		std::ostream& operator<<(const void* val);
};

#endif // __LOGGER_H__
