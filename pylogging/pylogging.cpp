#include <iostream>

#include <boost/python.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/slice.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/pure_virtual.hpp>

#include <log4cxx/logger.h>
#include <log4cxx/layout.h>

#include "logging_ctrl.h"

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
}

typedef return_value_policy<copy_const_reference> ccr;

BOOST_PYTHON_MODULE(pylogging)
{
	class_<log4cxx::Level, log4cxx::LevelPtr, boost::noncopyable>("LogLevel", no_init)
		.def("__eq__", eq)
		.def("__ne__", ne)
		.def("__gt__", gt)
		.def("__ge__", ge)
		.def("__lt__", lt)
		.def("__le__", le)
		.add_static_property("FATAL", getFatal)
		.add_static_property("ERROR", log4cxx::Level::getError)
		.add_static_property("WARN",  log4cxx::Level::getWarn)
		.add_static_property("INFO",  log4cxx::Level::getInfo)
		.add_static_property("DEBUG", log4cxx::Level::getDebug)
		.add_static_property("TRACE", log4cxx::Level::getTrace)
		.add_static_property("ALL",   log4cxx::Level::getAll)
	;

	class_<log4cxx::Logger, log4cxx::LoggerPtr, boost::noncopyable>("Logger", no_init)
		.def("TRACE", raw_function(LOG_TRACE, 1))
		.def("DEBUG", raw_function(LOG_DEBUG, 1))
		.def("INFO",  raw_function(LOG_INFO , 1))
		.def("WARN",  raw_function(LOG_WARN , 1))
		.def("ERROR", raw_function(LOG_ERROR, 1))
		.def("FATAL", raw_function(LOG_FATAL, 1))
	;

	def("reset", logger_reset, "Reset the logger config");

	def("config_from_file", logger_config_from_file,
			"Load logger config from the given configuration file");

	def("log_to_file", logger_log_to_file,
			"Configure the logger to log everything above the given loglevel to a file");

	def("log_to_cout", logger_log_to_cout,
			"Configure the logger to log everything above the given loglevel to stdout");

	def("set_loglevel", logger_set_loglevel,
			"Set the loglevel");

	def("get", get_logger, "Returns a logger for the given channel");

    def("LOG4CXX_TRACE", raw_function(LOG_TRACE, 1));
    def("LOG4CXX_DEBUG", raw_function(LOG_DEBUG, 1));
    def("LOG4CXX_INFO",  raw_function(LOG_INFO , 1));
    def("LOG4CXX_WARN",  raw_function(LOG_WARN , 1));
    def("LOG4CXX_ERROR", raw_function(LOG_ERROR, 1));
    def("LOG4CXX_FATAL", raw_function(LOG_FATAL, 1));
}
