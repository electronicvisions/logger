#include "logging_ctrl.h"

#include <stdexcept>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/propertyconfigurator.h>

#include <boost/filesystem.hpp>

#include "colorlayout.h"
#include "logger.h"

void logger_reset()
{
	log4cxx::BasicConfigurator::resetConfiguration();
}

void logger_config_from_file(std::string filename)
{
	using namespace boost::filesystem;
	path p = system_complete(path(filename));
	if (!exists(p))
	{
		std::stringstream err;
		err << "Logger configuration file '" << p.native() << "' could not be found";
		throw std::runtime_error(err.str());
	}
	log4cxx::PropertyConfigurator::configure(p.c_str());
}

log4cxx::AppenderPtr
logger_append_to_file(std::string filename, log4cxx::LoggerPtr logger)
{
	log4cxx::LayoutPtr layout(new log4cxx::ColorLayout(false));
	log4cxx::AppenderPtr appender(new log4cxx::FileAppender(
				layout, filename, false));
	logger->addAppender(appender);
	return appender;
}


log4cxx::AppenderPtr logger_append_to_cout(log4cxx::LoggerPtr logger)
{
	log4cxx::LayoutPtr layout(new log4cxx::ColorLayout());
	log4cxx::AppenderPtr appender(new log4cxx::ConsoleAppender(layout));
	logger->addAppender(appender);
	return appender;
}

void logger_log_to_file(std::string filename, log4cxx::LevelPtr level)
{
	logger_append_to_file(filename, log4cxx::Logger::getRootLogger());
	log4cxx::Logger::getRootLogger()->setLevel(level);
}

void logger_log_to_cout(log4cxx::LevelPtr level)
{
	logger_append_to_cout(log4cxx::Logger::getRootLogger());
	log4cxx::Logger::getRootLogger()->setLevel(level);
}

void logger_default_config(log4cxx::LevelPtr level,
		                   std::string fname, bool dual)
{
	using namespace boost::filesystem;

	get_log4cxx(level, fname, dual, false);

	if (fname.empty() && dual)
		throw std::logic_error("dual log mode requires a filename");

	path logger_config("symap2ic_logger.conf");
	if (exists(logger_config))
	{
		std::cerr << ">>>> config from: " << system_complete(logger_config).native() << std::endl;
		logger_config_from_file(system_complete(logger_config).native());
	}
	else
	{
		if (fname.empty() || dual)
		{
			std::cerr << ">>>> log to cout" << std::endl;
			logger_log_to_cout(level);
		}

		if (!fname.empty())
		{
			std::cerr << ">>>> log to " << fname << std::endl;
			logger_log_to_file(fname, level);
		}
	}
}


void logger_set_loglevel(log4cxx::LoggerPtr	l, log4cxx::LevelPtr level)
{
	l->setLevel(level);
}
