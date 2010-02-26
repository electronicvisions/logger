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
Every further call returns a reference to this one class. */

class DebugSingleton
{
private:
    uint dbglevel;
    std::string dbgfilename;
    std::ofstream *dbgfile;
    std::ostringstream dummy;
    static DebugSingleton *dbg;
    DebugSingleton(uint level, std::string filename)
    {
        dbgfile = 0;
        dbglevel = level;
        dbgfilename = filename;
    }
    DebugSingleton(DebugSingleton& ds)
    {
    }
    bool belowThresh(uint level)
    {
        return (level <= dbglevel);
    }

public:
    ~DebugSingleton()
    {
        if(dbgfile)
        {
            if(dbgfile->is_open()) dbgfile->close();
            delete dbgfile;
        }
    }
    
    enum levels {fatal=0,verbose=5,debug=10,all=99999};
    
    uint * getLevel() { return &dbglevel; }
    
    std::string getFilename() { return dbgfilename; }
    
    std::ostream & operator() (uint level=fatal)
    {
        if(belowThresh(level))
        {
            if(!dbgfile)
            {
                if(dbgfilename=="") return std::cout;
                else
                {
                    dbgfile = new std::ofstream;
                    dbgfile->open(dbgfilename.c_str(), std::ios::out);
                    *dbgfile << "";
                    dbgfile->close();
                    dbgfile->open(dbgfilename.c_str(), std::ios::app);
                }
            }
            return *dbgfile;
        }
        else
        {
            // make dummy string do nothing
            dummy.clear(std::ios::failbit);
            // provide dummy string, nothing will be done with it
            return dummy;
        }
    }
    
    static DebugSingleton& instance(uint level=fatal, std::string filename="")
    {
        if(!dbg) dbg = new DebugSingleton(level, filename);
        return *dbg;
    }
};

// Allocating and initializing DebugSingleton's
// static data members.  The pointer is being
// allocated - not the object inself.
DebugSingleton *DebugSingleton::dbg = 0;
