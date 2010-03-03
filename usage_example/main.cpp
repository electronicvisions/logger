#include "logger.h"

// this Logger will output its messages to stdout,
// but the empty string can be replaced by a filename
Logger& log = Logger::instance(Logger::INFO, "");

int main()
{
    log(Logger::DEBUG) << "This message won't be recorded by the logger, since its criticality level (DEBUG) is too low." << std::endl;
    log(Logger::INFO) << "IMMER LOGGER BLEIBEN!" << std::endl;
    log(Logger::WARNING) << "Something might have gone wrong." << std::endl;
    log(Logger::ERROR) << "Something went terribly wrong." << std::endl;
}
