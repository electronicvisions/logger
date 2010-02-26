// Singleton implementation of Debugger class for symap2ic project
// by Daniel Bruederle, bruederle@kip.uni-heidelberg.de
// 2010-02-26

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


typedef unsigned int uint;

//! Singleton implementation of Debugger class
/*! Only one single instance of this class can be created by calling the public function instance(). 
Every further call returns a reference to this one class. The parameters loglevel and logfile can 
not be changed after their first initialization. 
Pre-defined logging levels are: fatal=0,verbose=5,debug=10. */

class LoggerSingleton
{
private:
    uint loglevel; 
    std::string logfilename; 
    std::ofstream *logfile;
    std::ostringstream dummy;
    static LoggerSingleton *log;
    LoggerSingleton(uint level, std::string filename)
    {
        logfile = 0;
        loglevel = level;
        logfilename = filename;
    }
    LoggerSingleton(LoggerSingleton& ds)
    {
    }
    bool belowThresh(uint level)
    {
        return (level <= loglevel);
    }

public:
    ~LoggerSingleton()
    {
        if(logfile)
        {
            if(logfile->is_open()) logfile->close();
            delete logfile;
        }
    }
    
    enum levels {fatal=0,verbose=5,debug=10,all=99999};
    
    uint * getLevel() { return &loglevel; }
    
    std::string getFilename() { return logfilename; }
    
    std::ostream & operator() (uint level=fatal)
    {
        if(belowThresh(level))
        {
            if(!logfile)
            {
                if(logfilename=="") return std::cout;
                else
                {
                    logfile = new std::ofstream;
                    logfile->open(logfilename.c_str(), std::ios::out);
                    *logfile << "";
                    logfile->close();
                    logfile->open(logfilename.c_str(), std::ios::app);
                }
            }
            return *logfile;
        }
        else
        {
            // make dummy string do nothing
            dummy.clear(std::ios::failbit);
            // provide dummy string, nothing will be done with it
            return dummy;
        }
    }
    
    /*! This is the only way to create an instance of a LoggerSingleton. Only the first call actually creates an instance, 
    all further calls returns a reference to the one and only instance. */
    static LoggerSingleton& instance(
        uint level=fatal,         //! The logging threshold: every message with a level higher than this threshold will NOT be logged.
        std::string filename=""   //! The logging file: If nothing or an empty string is passed, std::cout is the default target for all outputs.
    )
    {
        if(!log) log = new LoggerSingleton(level, filename);
        return *log;
    }
};

// Allocating and initializing LoggerSingleton's static data member.  
// The pointer is allocated - not the object inself.
LoggerSingleton *LoggerSingleton::log = 0;
