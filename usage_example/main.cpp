#include "logger.h"

void other();
void another();

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

	// use flush if you want the logger to print to the file immediately
    log(Logger::ERROR) << "This message is written immediately" << Logger::flush;

	// the following line would cause an exception by design (exceptions can be handled)
    //log << "You can not write to an flushed stream instance anymore";

	// you need to get a new message instance by using the ()-operator
    log(Logger::INFO) << "New log stream instance. This message works again";

	{
		// new RAII-style feature: temporary log level escalation
		//    the local log level is reset within the object's current scope
		Logger::AlterLevel level_escalation(Logger::DEBUG0);
		log(Logger::DEBUG0) << "This message will be recorded, even though (DEBUG0) is globaly too low.";

		Logger::AlterLevel level_reduction(Logger::ERROR);
		log(Logger::WARNING) << "This message won't be recorded, even though (WARNING) is globaly high enough.";
	}

#ifdef LOG_COLOR_OUTPUT
	// you can use colors, if you turn them of during configuration
	log(Logger::INFO) << "R" << LogStream::purple;
	log << "a" << LogStream::red;
	log << "i" << LogStream::yellow;
	log << "n" << LogStream::green;
	log << "b" << LogStream::marine;
	log << "o" << LogStream::blue;
	log << "w" << LogStream::purple;
#endif // LOG_COLOR_OUTPUT

	another();
}


void other()
{
    // this is how in an arbitrary scope a reference to the one Logger instance 
    // initialized in main() is retrieved
    Logger& log = Logger::instance();

    log(Logger::INFO) << "This message comes from a different scope but goes into the same Logger instance.";
}

void another() {
	// logging with a prefix...
	PrefixedLogger& log = PrefixedLogger::instance("myprefix", Logger::INFO);

	log(Logger::INFO) << "This is a prefixed test";
	log(Logger::INFO) << "This is another prefixed test";
}
