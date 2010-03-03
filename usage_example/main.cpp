#include "logger.h"


void other();

int main()
{
    // this Logger will output its messages to stdout,
    // but the empty string can be replaced by a filename
    Logger& log = Logger::instance(Logger::INFO, "");

    log(Logger::DEBUG) << "This message won't be recorded by the logger, since its criticality level (DEBUG) is too low." << std::endl;
    log(Logger::INFO) << "IMMER LOGGER BLEIBEN!" << std::endl;
    other();
    log(Logger::WARNING) << "Something might have gone wrong." << std::endl;
    log(Logger::ERROR) << "Something went terribly wrong." << std::endl;
}


void other()
{
    // this is how in an arbitrary scope a reference to the one Logger instance 
    // initialized in main() is retrieved
    Logger& log = Logger::instance();

    log(Logger::INFO) << "This message comes from a different scope but goes into the same Logger instance." << std::endl;
}
