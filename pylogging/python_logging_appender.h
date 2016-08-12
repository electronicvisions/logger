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

#pragma once

/**
 * @file python_logging_appender.hpp
 *
 * This file contains a simple appender class which forwards log4cxx log
 * messages to the Python logging module via boost::python. This allows central
 * handling of logging.
 *
 * @author Andreas Stöckel
 */

#include <memory>
#include <string>

#include <log4cxx/appenderskeleton.h>

namespace log4cxx {
/*
 * Forward declarations.
 */
class PythonLoggingAppenderImpl;

/**
 * Implements an adapter class which write log4cxx events to the Python
 * "logging" class. Note that boost::python needs to be initialized before this
 * class is used.
 */
class PythonLoggingAppender : public AppenderSkeleton {
private:
	std::unique_ptr<PythonLoggingAppenderImpl> m_impl;

protected:
	/**
	 * Called whenever a log message is received. Forwards the log message to
	 * Python.
	 */
	void append(const spi::LoggingEventPtr &event,
	            log4cxx::helpers::Pool &) override;

public:
	/**
	 * Constructor of the PythonLoggingAppender class.
	 *
	 * @param domain is the name of the Python logger to which log messages
	 * should be written. Per default an empty string is passed, which
	 * corresponds to the root logger.
	 */
	PythonLoggingAppender(const std::string &domain = std::string());

	/**
	 * Destructor of the PythonLoggingAppender class. Cleans up the internal
	 * implementation.
	 */
	~PythonLoggingAppender() override;

	/**
	 * Releases any pointers at Python objects.
	 */
	void close() override;

	/**
	 * Used internally by log4cxx, indicates that the PythonLoggingAppender
	 * class does not require a layout manager.
	 */
	bool requiresLayout() const override { return false; }
};
}
