/* log4cxx-based logger needs cxx lib, nothing else */

#include "logging_ctrl.h"
#include <stdexcept>
#include <iostream>

namespace {
	void configure_default_logger(log4cxx::LoggerPtr logger, log4cxx::LevelPtr level, std::string fname, bool dual)
	{
		if (fname.empty() && dual)
			throw std::logic_error("dual log mode requires a filename");

		logger->setLevel(level);

		if (fname.empty() || dual)
		{
			logger_append_to_cout(logger);
		}

		if (!fname.empty())
		{
			logger_append_to_file(fname, logger);
		}
	}
} // end namespace

log4cxx::LoggerPtr get_default_logger(log4cxx::LevelPtr level, std::string fname, bool dual)
{
	log4cxx::LoggerPtr new_logger = log4cxx::Logger::getLogger("Default");
	// Don't spam the root logger
	new_logger->setAdditivity(false);
	if (new_logger->getAllAppenders().size() == 0)
		configure_default_logger(new_logger, level, fname, dual);
	return new_logger;
}

