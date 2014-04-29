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
	log4cxx::FileAppenderPtr appender(new log4cxx::FileAppender(
				layout, filename, false));
	appender->setImmediateFlush(true);
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

log4cxx::AppenderPtr logger_log_to_file(
		std::string filename, log4cxx::LevelPtr level)
{
	log4cxx::AppenderPtr appender = logger_append_to_file(
			filename, log4cxx::Logger::getRootLogger());
	log4cxx::Logger::getRootLogger()->setLevel(level);
	return appender;
}

log4cxx::AppenderPtr logger_log_to_cout(log4cxx::LevelPtr level)
{
	log4cxx::AppenderPtr appender = logger_append_to_cout(
			log4cxx::Logger::getRootLogger());
	log4cxx::Logger::getRootLogger()->setLevel(level);
	return appender;
}

void logger_default_config(
		log4cxx::LevelPtr level, std::string fname, bool dual,
		bool print_location, bool use_color, std::string date_format)
{
	using namespace boost::filesystem;

	path logger_config("symap2ic_logger.conf");
	if (exists(logger_config))
	{
		logger_config_from_file(system_complete(logger_config).native());
	}
	else
	{
		configure_default_logger(log4cxx::Logger::getRootLogger(),
				level, fname, dual);

		log4cxx::helpers::Pool pool;
		log4cxx::AppenderList list =
			log4cxx::Logger::getRootLogger()->getAllAppenders();
		for (log4cxx::AppenderList::iterator it = list.begin(), end = list.end();
				it != end; ++it)
		{
			log4cxx::LayoutPtr layout = (*it)->getLayout();
			layout->setOption("color", use_color ? "true" : "false");
			layout->setOption("printlocation", print_location ? "true" : "false");
			layout->setOption("dateformat", date_format);
			layout->activateOptions(pool);
		}
	}
}



void logger_set_loglevel(log4cxx::LoggerPtr	l, log4cxx::LevelPtr level)
{
	l->setLevel(level);
}
