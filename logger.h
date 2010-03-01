// Singleton implementation of Debugger class for symap2ic project
// by Daniel Bruederle, bruederle@kip.uni-heidelberg.de
// by Sebastian Jeltsch, sjeltsch@kip.uni-heidelberg.de
// 2010-03-01

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

class LoggerSingleton;
class LoggerSingleton
{
private:
    size_t loglevel;
    std::string logfilename;
    std::fstream* logfile;
	std::ostringstream deafStream;
    static boost::shared_ptr<LoggerSingleton> log_ptr;
    LoggerSingleton(size_t level, std::string filename)
    {
        logfile = 0;
        loglevel = level;
        logfilename = filename;
    }
    LoggerSingleton(LoggerSingleton&) {}
    bool belowThresh(size_t level)
    {
        return level <= loglevel ? true : false;
    }

public:
    ~LoggerSingleton()
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
    
    size_t* getLevel() { return &loglevel; }
    
    std::string getFilename() { return logfilename; }
    
    std::ostream& operator() (size_t level=DEBUG)
    {
        if(belowThresh(level))
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

	std::ostream& operator<< (std::string msg)
	{
		return (*this)(DEBUG) << msg;
	}

    /*! This is the only way to create an instance of a LoggerSingleton. Only the first call actually creates an instance, 
    all further calls returns a reference to the one and only instance. */
    static LoggerSingleton& instance(
        size_t level=FATAL,         //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
        std::string filename=""   //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
    )
    {
        if(!log_ptr) log_ptr = boost::shared_ptr<LoggerSingleton> (new LoggerSingleton(level, filename));
        return *log_ptr;
    }
};

// Allocating and initializing LoggerSingleton's static data member.  
// The smart pointer is allocated - not the object inself.
boost::shared_ptr<LoggerSingleton> LoggerSingleton::log_ptr;
