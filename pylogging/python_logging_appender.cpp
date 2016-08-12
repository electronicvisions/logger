/*
 *  log4cxx Python Logging Appender
 *  Copyright (C) 2016  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>

#include <boost/python.hpp>

#include "python_logging_appender.h"

namespace log4cxx {

/**
 * Actual implementation of the PythonLoggingAppender. This class is responsible
 * for actually sending the log messages to the Python "logging" module. This
 * module is lazily loaded once the first log message is received.
 */
class PythonLoggingAppenderImpl {
private:
	std::string m_domain;
	boost::python::object m_logger;

	/**
	 * Initializes the m_logger variable if this has not been done yet.
	 */
	void init()
	{
		// Abort if the logger has already been fetched
		if (m_logger.ptr() != boost::python::object().ptr()) {
			return;
		}

		// Place the domain name in the local variable dictionary
		boost::python::dict locals;
		locals["domain"] = m_domain;

		// Fetch the logger
		boost::python::exec(
		    "import logging\nlogger = logging.getLogger(domain)\n",
		    boost::python::object(), locals);
		m_logger = locals["logger"];
	}

	/**
	 * Lossily converts log4cxx loglevels to corresponding Python log levels.
	 *
	 * @param level is the log4cxx log level.
	 * @return a corresponding Python log level.
	 */
	static int convert_level(int level)
	{
		if (level >= Level::FATAL_INT) {
			return 50;  // CRITICAL
		}
		else if (level >= Level::ERROR_INT) {
			return 40;  // ERROR
		}
		else if (level >= Level::WARN_INT) {
			return 30;  // WARNING
		}
		else if (level >= Level::INFO_INT) {
			return 20;  // INFO
		}
		else if (level >= Level::DEBUG_INT) {
			return 10;  // DEBUG
		}
		return 10;  // DEBUG
	}

public:
	PythonLoggingAppenderImpl(const std::string &domain) : m_domain(domain) {}

	void append(const spi::LoggingEventPtr &event)
	{
		// Initialize logging
		init();

		// Place all variables in the local variable dictionary
		boost::python::dict locals;
		locals["logger"] = m_logger;
		locals["level"] = convert_level(event->getLevel()->toInt());
		locals["message"] = event->getMessage();

		// Get the logger name, if it is empty, directly write to the logger
		// obtained by init(), otherwise write to a child logger.
		std::string logger_name = event->getLoggerName();
		if (!logger_name.empty()) {
			locals["context"] = logger_name;
			boost::python::exec("logger.getChild(context).log(level, message);",
			                    boost::python::object(), locals);
		}
		else {
			boost::python::exec("logger.log(level, message);",
			                    boost::python::object(), locals);
		}
	}

	void close()
	{
		// Delete the reference to the logger
		m_logger = boost::python::object();
	}
};

PythonLoggingAppender::PythonLoggingAppender(const std::string &domain)
    : m_impl(new PythonLoggingAppenderImpl(domain))
{
}

PythonLoggingAppender::~PythonLoggingAppender()
{
	// Do nothing here, only needed for the unique_ptr to be properly deleted
}

void PythonLoggingAppender::append(const spi::LoggingEventPtr &event,
                                   log4cxx::helpers::Pool &)
{
	m_impl->append(event);
}

void PythonLoggingAppender::close() { m_impl->close(); }
}
