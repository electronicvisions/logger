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
Pre-defined logging levels are: FATAL=0,VERBOSE=5,DEBUG=10.
Default log level ist DEBUG if you reference the the debugger wit no explicit log level */

class Logger;
class Logger
{
private:
    size_t loglevel;
    std::string logfilename;
    std::fstream* logfile;
	std::ostringstream deafStream;
    static boost::shared_ptr<Logger> log_ptr;
    Logger(size_t level, std::string filename)
    {
        logfile = 0;
        loglevel = level;
        logfilename = filename;
    }
    Logger(Logger&) {}
    bool belowThresh(size_t level)
    {
        return level <= loglevel ? true : false;
    }

public:
    ~Logger()
    {
        if(logfile)
        {
            if(logfile->is_open())
			{
				logfile->flush();
				logfile->close();
			}
            delete logfile;
        }
    }
    
    enum levels {FATAL=0,VERBOSE=5,DEBUG=10,ALL=99999};
    
    size_t getLevel() { return loglevel; }
    
    //! tells if the passed logging level is below the logger's threshold
    bool willBeLogged(unsigned int level)
    {
        return (level <= loglevel);
    }
    //! Identical to willBeLogged(), for backward compatibility. 
    bool l(unsigned int level)
    {
        return willBeLogged(level);
    }
    std::string getFilename() { return logfilename; }
    
    std::ostream& operator() (size_t level=DEBUG)
    {
        if(willBeLogged(level))
        {
            if(!logfile)
            {
                if(logfilename=="") return std::cout;
                else
                {
					// erase existing log file if necessary
                    logfile = new std::fstream;
                    logfile->open(logfilename.c_str(), (std::fstream::out | std::fstream::binary) );
                    *logfile << "";
                    logfile->close();

					// open clean log file in append mode
                    logfile->open(logfilename.c_str(), (std::fstream::app | std::fstream::out | std::fstream::binary) );
                }
            }
            return *logfile;
        }
        else
        {
			// return deaf stream
			deafStream.clear();
            return deafStream;
        }
    }

	inline std::ostream& operator<<(std::basic_string<char> val)    { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(std::ostringstream& val)        { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(long val)                       { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(unsigned long val)              { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(bool val)                       { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(short val)                      { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(unsigned short val)             { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(int val)                        { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(unsigned int val)               { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(long long val)                  { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(unsigned long long val)         { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(double val)                     { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(float val)                      { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(long double val)                { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(const char* val)                { return (*this)(DEBUG) << val; }
	inline std::ostream& operator<<(const void* val)                { return (*this)(DEBUG) << val; }

	/*! This is the only way to create an instance of a Logger. Only the first call actually creates an instance, 
	  all further calls returns a reference to the one and only instance. */
	static Logger& instance(
			size_t level=FATAL,         //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
			std::string filename=""     //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
			)
	{
		if(!log_ptr) log_ptr = boost::shared_ptr<Logger> (new Logger(level, filename));
		return *log_ptr;
	}
};

#ifndef PYTHONEXPOSED
// Allocating and initializing Logger's static data member.  
// The smart pointer is allocated - not the object inself.
boost::shared_ptr<Logger> Logger::log_ptr;
#endif //PYTHONEXPOSED

#endif // __LOGGER_H__
