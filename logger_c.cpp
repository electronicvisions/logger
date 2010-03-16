#include "logger.h"
#include "logger_c.h"

void init(unsigned int level, const char* filename, int dual)
{
	Logger::instance(level, filename, dual);
}

void toLog(unsigned int level, const char* msg) {
	Logger& dbg = Logger::instance();
	dbg(level) << msg;
}
