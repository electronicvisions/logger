#include "logger.h"


void other();

int main()
{
    // this Logger will output its messages to stdout,
    // but the empty string can be replaced by a filename
	// messages are auto-endlined. Therefore you don't need to stream a std::endl
    Logger& log = Logger::instance(Logger::INFO, "");

    log(Logger::DEBUG0) << "This message won't be recorded by the logger, since its criticality level (DEBUG0) is too low.";
    log(Logger::INFO) << "IMMER LOGGER BLEIBEN!";
    other();
    log(Logger::WARNING) << "Something might have gone wrong.";
    log(Logger::ERROR) << "Something went terribly wrong.";

	// multi-line messages are also possible
	log(Logger::INFO) << "First line of multi-line message." << std::endl << "    Second line of multi-line message.\n";
	log << "    Third line of multi-line message.";
}


void other()
{
    // this is how in an arbitrary scope a reference to the one Logger instance 
    // initialized in main() is retrieved
    Logger& log = Logger::instance();

    log(Logger::INFO) << "This message comes from a different scope but goes into the same Logger instance.";
}
