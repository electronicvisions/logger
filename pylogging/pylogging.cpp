#include <iostream>

#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/pure_virtual.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/layout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/fileappender.h>

#include <log4cxx/filter/levelrangefilter.h>

#include "logging_ctrl.h"
#include "logger.h"
#include "colorlayout.h"
#include "python_logging_appender.h"

using namespace boost::python;

namespace boost {
namespace python {

template <class T> struct pointee<log4cxx::helpers::ObjectPtrT<T> >
{
	typedef T type;
};

} // end namespace python
} // end namespace boost

namespace log4cxx {
namespace helpers {

template <typename T>
inline T * get_pointer(const ObjectPtrT<T> & ptr)
{
	return static_cast<T*>(ptr);
}

} // namespace helpers
} // namespace log4cxx

#include "pylogging_wrapper.h"

namespace {
	log4cxx::LevelPtr getFatal() { return log4cxx::Level::getFatal(); }

	bool eq(log4cxx::LevelPtr a, log4cxx::LevelPtr b) { return *a == *b; }
	bool ne(log4cxx::LevelPtr a, log4cxx::LevelPtr b) { return *a != *b; }
	bool gt(log4cxx::LevelPtr, log4cxx::LevelPtr) { throw std::runtime_error("No order defined on loglevels"); }
	bool ge(log4cxx::LevelPtr, log4cxx::LevelPtr) { throw std::runtime_error("No order defined on loglevels"); }
	bool lt(log4cxx::LevelPtr, log4cxx::LevelPtr) { throw std::runtime_error("No order defined on loglevels"); }
	bool le(log4cxx::LevelPtr, log4cxx::LevelPtr) { throw std::runtime_error("No order defined on loglevels"); }

	object log(log4cxx::LevelPtr level, tuple args)
	{
		log4cxx::LoggerPtr logger = extract<log4cxx::LoggerPtr>(args[0]);
		// Simulate: LOG4CXX_LOG macro
		if (logger->isEnabledFor(level))
		{
			object message_args = args[slice(1, slice_nil())];
			::log4cxx::helpers::LogCharMessageBuffer oss_;

			object inspect = import("inspect");
			object frameinfo = inspect.attr("getframeinfo")(inspect.attr("currentframe")());
			std::string filename = extract<std::string>(frameinfo.attr("filename"));
			std::string function = extract<std::string>(frameinfo.attr("function"));
			int lineno = extract<int>(frameinfo.attr("lineno"));
			log4cxx::spi::LocationInfo location(filename.c_str(), function.c_str(), lineno);

			stl_input_iterator<object> it(args), end;
			++it; // Skip logger
			for(; it != end; ++it)
			{
				std::string value = extract<std::string>(str(*it));
				oss_ << value;
			}
			logger->forcedLog(level, oss_.str(oss_), location);
		}
		return object();
	}

	log4cxx::LevelPtr FATAL = log4cxx::Level::getFatal();
	log4cxx::LevelPtr ERROR = log4cxx::Level::getError();
	log4cxx::LevelPtr WARN  = log4cxx::Level::getWarn();
	log4cxx::LevelPtr INFO  = log4cxx::Level::getInfo();
	log4cxx::LevelPtr DEBUG = log4cxx::Level::getDebug();
	log4cxx::LevelPtr TRACE = log4cxx::Level::getTrace();

	object LOG_FATAL (tuple args, dict) { return log(FATAL, args); }
	object LOG_ERROR (tuple args, dict) { return log(ERROR, args); }
	object LOG_WARN  (tuple args, dict) { return log(WARN , args); }
	object LOG_INFO  (tuple args, dict) { return log(INFO , args); }
	object LOG_DEBUG (tuple args, dict) { return log(DEBUG, args); }
	object LOG_TRACE (tuple args, dict) { return log(TRACE, args); }

	log4cxx::LoggerPtr get_logger(std::string channel) { return log4cxx::Logger::getLogger(channel); }
	log4cxx::LoggerPtr get_root_logger() { return log4cxx::Logger::getRootLogger(); }
	log4cxx::LoggerPtr get_old_logger(log4cxx::LevelPtr level, std::string file, bool dual) {
			log4cxx::Logger & l = get_log4cxx(level, "PyLogging", file, dual);
			return log4cxx::LoggerPtr(&l);
	}

	size_t get_number_of_appenders(log4cxx::LoggerPtr logger) {
		return logger->getAllAppenders().size();
	}


	void activateOptionsHelper(log4cxx::spi::OptionHandler & handler)
	{
		log4cxx::helpers::Pool pool;
		handler.activateOptions(pool);
	}


	/// adds a ConsoleAppender to the given logger
	log4cxx::AppenderPtr logger_write_to_logging(
		std::string const &domain,
		log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger())
	{
		// Check whether a logger for this domain was already added -- if yes,
		// do nothing. It makes no sense to log to the same logging domain
		// multiple times
		for (log4cxx::AppenderPtr &ptr : logger->getAllAppenders()) {
			log4cxx::PythonLoggingAppender *pyapp =
				dynamic_cast<log4cxx::PythonLoggingAppender*>(&*ptr);
			if (pyapp && pyapp->domain() == domain) {
				return ptr;
			}
		}

		log4cxx::AppenderPtr appender(new log4cxx::PythonLoggingAppender(domain));
		logger->addAppender(appender);
		return appender;
	}
}

class PyWriter : public log4cxx::helpers::Writer
{
	void close(log4cxx::helpers::Pool &) {}
	void flush(log4cxx::helpers::Pool &) {}
	void write(const log4cxx::LogString &str, log4cxx::helpers::Pool &)
	{
		PySys_WriteStdout(str.c_str());
	}
};

LOG4CXX_PTR_DEF(PyWriter);

typedef return_value_policy<copy_const_reference> ccr;
typedef return_value_policy<reference_existing_object> reo;

BOOST_PYTHON_MODULE(pylogging)
{
	class_<log4cxx::helpers::Pool, boost::noncopyable>(
			"Pool")
	;

	class_<PyWriter, PyWriterPtr, boost::noncopyable>(
			"PyWriter",
			"Writer class that writes to python stdout.\n"
			"Example:\n"
			"    appender = pylogging.log_to_cout(pylogging.LogLevel.WARN)\n"
			"    appender.setWriter(pylogging.PyWriter())\n",
			init<>())
	;
	implicitly_convertible<PyWriterPtr, log4cxx::helpers::WriterPtr>();

	class_<log4cxx::spi::OptionHandler, log4cxx::spi::OptionHandlerPtr, boost::noncopyable>(
			"OptionHandler", no_init)
		.def("activateOptions", &log4cxx::spi::OptionHandler::activateOptions)
		.def("activateOptions", activateOptionsHelper)
		.def("setOption", &log4cxx::spi::OptionHandler::setOption)
	;

	class_<log4cxx::Level, log4cxx::LevelPtr, boost::noncopyable>("LogLevel", no_init)
		.def("__eq__", eq)
		.def("__ne__", ne)
		.def("__gt__", gt)
		.def("__ge__", ge)
		.def("__lt__", lt)
		.def("__le__", le)
		.add_static_property("FATAL", log4cxx::Level::getFatal)
		.add_static_property("ERROR", log4cxx::Level::getError)
		.add_static_property("WARN",  log4cxx::Level::getWarn)
		.add_static_property("INFO",  log4cxx::Level::getInfo)
		.add_static_property("DEBUG", log4cxx::Level::getDebug)
		.add_static_property("TRACE", log4cxx::Level::getTrace)
		.add_static_property("ALL",   log4cxx::Level::getAll)
		.def("toLevel",
			static_cast<log4cxx::LevelPtr (*)(const std::string&, const log4cxx::LevelPtr &)>(log4cxx::Level::toLevel),
			"Convert the string passed as argument to a level. If the "
			"conversion fails, then this method returns the value of defaultLevel.")
		.staticmethod("toLevel")
		.def("toString",
			static_cast<log4cxx::LogString (log4cxx::Level::*)() const>(&log4cxx::Level::toString))
	;

	class_<log4cxx::Logger, log4cxx::LoggerPtr, boost::noncopyable>("Logger", no_init)
		.def("TRACE", raw_function(LOG_TRACE, 1))
		.def("DEBUG", raw_function(LOG_DEBUG, 1))
		.def("INFO",  raw_function(LOG_INFO , 1))
		.def("WARN",  raw_function(LOG_WARN , 1))
		.def("ERROR", raw_function(LOG_ERROR, 1))
		.def("FATAL", raw_function(LOG_FATAL, 1))
		.def("trace", raw_function(LOG_TRACE, 1))
		.def("debug", raw_function(LOG_DEBUG, 1))
		.def("info",  raw_function(LOG_INFO , 1))
		.def("warn",  raw_function(LOG_WARN , 1))
		.def("error", raw_function(LOG_ERROR, 1))
		.def("fatal", raw_function(LOG_FATAL, 1))
		.def("addAppender", &log4cxx::Logger::addAppender, "Add newAppender to the list of appenders of this Logger instance.\n"
				                                           "If newAppender is already in the list of appenders, then it won't be added again.")
		.def("setAdditivity", &log4cxx::Logger::setAdditivity, "Set the additivity flag for this Logger instance.")
		.def("getName", static_cast<const log4cxx::LogString (log4cxx::Logger::*)() const>(&log4cxx::Logger::getName),
			 "Get the logger name.")
		.def("_get_number_of_appenders", get_number_of_appenders, "for debug/test use")
	;

	class_<log4cxx::Layout, log4cxx::LayoutPtr, boost::noncopyable,
		bases<log4cxx::spi::OptionHandler> >("Layout", no_init)
	;

	class_<log4cxx::ColorLayout, log4cxx::ColorLayoutPtr, boost::noncopyable,
		bases<log4cxx::Layout> >(
			"ColorLayout", init< optional<bool, log4cxx::LogString> >())
	;
	implicitly_convertible< log4cxx::ColorLayoutPtr, log4cxx::LayoutPtr>();

	class_<log4cxx::Appender, log4cxx::AppenderPtr, boost::noncopyable,
		bases<log4cxx::spi::OptionHandler> >(
			"Appender", no_init)
		.def("addFilter", &log4cxx::Appender::addFilter, "Add a filter to the end of the filter list.")
	;

	class_<log4cxx::ConsoleAppender,
		   log4cxx::ConsoleAppenderPtr,
		   boost::noncopyable,
		   bases<log4cxx::Appender> >(
			"ConsoleAppender", init<log4cxx::LayoutPtr>())
		.def("setTarget", &log4cxx::ConsoleAppender::setTarget,
				"Sets the value of the target property. Recognized values "
				"are \"System.out\" and \"System.err\". Any other value will be "
				"ignored.")
		.def("setWriter", &log4cxx::ConsoleAppender::setWriter)
		.def("getSystemOut", &log4cxx::ConsoleAppender::getSystemOut, ccr()).staticmethod("getSystemOut")
		.def("getSystemErr", &log4cxx::ConsoleAppender::getSystemErr, ccr()).staticmethod("getSystemErr")
	;
	implicitly_convertible< log4cxx::ConsoleAppenderPtr, log4cxx::AppenderPtr>();

	class_<log4cxx::FileAppender,
		   log4cxx::FileAppenderPtr,
		   boost::noncopyable,
		   bases<log4cxx::Appender> >(
			"FileAppender", init<log4cxx::LayoutPtr, std::string, bool>())
		.def("setFile",	static_cast<void (::log4cxx::FileAppender::*)(const log4cxx::LogString&)>(&log4cxx::FileAppender::setFile), "Set file name of FileAppender.")
	;
	implicitly_convertible< log4cxx::FileAppenderPtr, log4cxx::AppenderPtr>();

	class_<log4cxx::spi::Filter,
		   log4cxx::spi::FilterPtr,
		   boost::noncopyable,
		   bases<log4cxx::spi::OptionHandler> >("Filter", no_init)
	;

	class_<log4cxx::filter::LevelRangeFilter,
	       log4cxx::filter::LevelRangeFilterPtr,
		   boost::noncopyable, bases<log4cxx::spi::Filter> >(
			"LevelRangeFilter", init<>())
		.def("setLevelMax",	     &log4cxx::filter::LevelRangeFilter::setLevelMax)
		.def("setLevelMin",      &log4cxx::filter::LevelRangeFilter::setLevelMin)
		.def("setAcceptOnMatch", &log4cxx::filter::LevelRangeFilter::setAcceptOnMatch, ccr())
		.def("getLevelMax",      &log4cxx::filter::LevelRangeFilter::getLevelMax, ccr())
		.def("getLevelMin",      &log4cxx::filter::LevelRangeFilter::getLevelMin, ccr())
		.def("getAcceptOnMatch", &log4cxx::filter::LevelRangeFilter::getAcceptOnMatch)
	;
	implicitly_convertible< log4cxx::filter::LevelRangeFilterPtr, log4cxx::spi::FilterPtr>();

	def("reset", logger_reset, "Reset the logger config");

	def("default_config", logger_default_config,
			( arg("level") = log4cxx::Level::getWarn(),
			  arg("fname")="",
			  arg("dual")=false,
			  arg("print_location")=false,
			  arg("color")=true,
			  arg("date_format")="ABSOLUTE"),
		"This is the default configuration procedure for the logger\n"
		"If the file 'symap2ic_logger.conf' is found, it is used to configure the\n"
		"logger and every other argument is ignored!\n"
		"@level: Log level\n"
		"@arg fname: Log to this file, if empty to stdout\n"
		"@arg dual: If file is given, log also to stdout\n"
		"@print_location: Include location of error into log message\n"
		"@use_color: Print colorfull\n"
		"@arg date_format: values are: NULL, RELATIVE, ABSOLUTE, DATE, ISO8601\n");

	def("config_from_file", logger_config_from_file,
			"Load logger config from the given configuration file");

	def("append_to_file", logger_append_to_file,
	    (arg("filename"), arg("logger") = log4cxx::Logger::getRootLogger()),
	    "adds a FileAppender to the given logger");

	def("write_to_file", logger_write_to_file,
	    (arg("filename"), arg("append") = false, arg("logger") = log4cxx::Logger::getRootLogger()),
	    "adds a FileAppender to the given logger");

	def("write_to_cout", logger_write_to_cout, (arg("logger") = log4cxx::Logger::getRootLogger()),
	    "adds a ConsoleAppender to the given logger");

	def("append_to_cout", logger_write_to_cout, (arg("logger") = log4cxx::Logger::getRootLogger()),
	    "adds a ConsoleAppender to the given logger");

	def("write_to_logging", logger_write_to_logging, (arg("domain"), arg("logger") = log4cxx::Logger::getRootLogger()),
	    "adds a PythonLoggingAppender to the given logger");

	def("append_to_logging", logger_write_to_logging, (arg("domain"), arg("logger") = log4cxx::Logger::getRootLogger()),
	    "adds a PythonLoggingAppender to the given logger");

	def("log_to_file", logger_log_to_file,
			"Configure the logger to log everything above the given loglevel to a file");

	def("log_to_cout", logger_log_to_cout,
			"Configure the logger to log everything above the given loglevel to stdout");

	def("set_loglevel", logger_set_loglevel,
			"Set the loglevel");

	def("get", get_logger, "Returns a logger for the given channel");
	def("get_root", get_root_logger, "Returns a logger for the given channel");
	def("get_old_logger", get_old_logger,
			(arg("level") = Logger::log4cxx_level(LOGGER_DEFAULT_LEVEL), arg("file") = "", arg("dual") = false),
			"Returns the old style default logger, usage is deprecated");

    def("LOG4CXX_TRACE", raw_function(LOG_TRACE, 1));
    def("LOG4CXX_DEBUG", raw_function(LOG_DEBUG, 1));
    def("LOG4CXX_INFO",  raw_function(LOG_INFO , 1));
    def("LOG4CXX_WARN",  raw_function(LOG_WARN , 1));
    def("LOG4CXX_ERROR", raw_function(LOG_ERROR, 1));
    def("LOG4CXX_FATAL", raw_function(LOG_FATAL, 1));
}
