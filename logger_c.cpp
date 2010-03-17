#include "logger.h"
#include "logger_c.h"

void log_init(unsigned int level, const char* filename, int dual)
{
	Logger::instance(level, filename, dual);
}

void log_print(unsigned int level, const char* msg) {
	Logger& dbg = Logger::instance();
	dbg(level) << msg;
}
