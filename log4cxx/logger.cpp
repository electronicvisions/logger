/* log4cxx-based logger needs cxx lib, nothing else */

#include <stdexcept>
#include <iostream>

extern "C" {
#include <execinfo.h>
#include <syslog.h>
#include <unistd.h>
}

#include "logging_ctrl.h"

namespace visionary_logger {

std::string print_backtrace() {
	std::stringstream ret;
	ret << "print_trace: Printing stack backtrace\n";

	void* array[10];
	size_t size = backtrace(array, 10);
	char** strings = backtrace_symbols(array, size);

	for (size_t i = 0; i < size; i++)
		ret << "print_trace: " << strings[i] << "\n";

	return ret.str();
}

std::string make_syslog_prefix() {
	std::stringstream ret;
	ret << "vislog"
	    << " uid=" << getuid()
		<< " pid=" << getpid();

	char hostname[_POSIX_HOST_NAME_MAX + 1];
	if (gethostname(hostname, sizeof hostname) == 0 ) {
		ret << " hostname=" << hostname;
	}
	char * slurm_job_id = getenv("SLURM_JOBID");
	if (slurm_job_id != nullptr ) {
		ret << " SLURM_JOBID=" << slurm_job_id;
	}
	ret << "|";
	return ret.str();
}

void write_to_syslog(std::string message) {
	static const std::string prefix = make_syslog_prefix();
	syslog(LOG_ERR, (prefix + message).c_str());
}

} // namespace visionary_logger

void configure_default_logger(log4cxx::LoggerPtr logger,
		log4cxx::LevelPtr level, std::string fname, bool dual)
{
	if (fname.empty() && dual)
		throw std::logic_error("dual log mode requires a filename");

	logger->setLevel(level);

	if (fname.empty() || dual)
	{
		log4cxx::AppenderPtr app = logger_write_to_cout(logger);
		app->setName("COUT");
	}

	if (!fname.empty())
	{
		log4cxx::AppenderPtr app = logger_write_to_file(fname, logger);
		app->setName("FILE");
	}
}

log4cxx::LoggerPtr get_default_logger(std::string logger_name, log4cxx::LevelPtr level, std::string fname, bool dual)
{
	log4cxx::LoggerPtr new_logger = log4cxx::Logger::getLogger(logger_name);
	if (log4cxx::Logger::getRootLogger()->getAllAppenders().size() == 0
			&& new_logger->getAllAppenders().size() == 0)
	{
		// TODO Don't spam the root logger ?
		// new_logger->setAdditivity(false);
		configure_default_logger(new_logger, level, fname, dual);
	}
	return new_logger;
}

