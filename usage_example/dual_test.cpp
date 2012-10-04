#include "logger.h"

int main()
{

    Logger& log = Logger::instance(Logger::INFO, "logfile.txt",/* dual */ true);

    log(Logger::INFO) << "Example of Logger in Dual Mode:";
    log(Logger::INFO) << "Logger streams to file(logfile.txt)";
    log(Logger::INFO) << "... and to stdout";
}
