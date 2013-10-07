#include "logging_ctrl.h"

#include <stdexcept>

#include <log4cxx/basicconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/propertyconfigurator.h>

#include <boost/filesystem.hpp>

#include "colorlayout.h"

using namespace log4cxx;

void logger_reset()
{
	BasicConfigurator::resetConfiguration();
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

void logger_log_to_file(std::string filename, log4cxx::LevelPtr level)
{
	log4cxx::FileAppender* file = new log4cxx::FileAppender(
			log4cxx::LayoutPtr(new log4cxx::ColorLayout(false)), filename, false);
	log4cxx::BasicConfigurator::configure(log4cxx::AppenderPtr(file));
	Logger::getRootLogger()->setLevel(level);
}

void logger_log_to_cout(log4cxx::LevelPtr level)
{
	log4cxx::LayoutPtr layout(new log4cxx::ColorLayout());
	log4cxx::ConsoleAppender* console = new log4cxx::ConsoleAppender(layout);
	log4cxx::BasicConfigurator::configure(log4cxx::AppenderPtr(console));
	Logger::getRootLogger()->setLevel(level);
}


void logger_set_loglevel(log4cxx::LoggerPtr	l, log4cxx::LevelPtr level)
{
	l->setLevel(level);
}
