#pragma once
#include <sstream>
#include <cassert>
#include <string>

#include <log4cxx/logger.h>
#include <log4cxx/layout.h>

/// The functions in this file are no tintentended be used in library code.
/// Only use it in front-end code, tools or test-runners to allow the user to
/// choose a resonable logging behaviour. Generelly spoken it is ok to use it
/// in main() otherwise not!
/// By using this functions you agree, that you wont bother other people by
/// missusing this functions and accept any punishment the Softie-court will
/// give you in any case of misusage.

/// Note: you can always use the log4cxx api directly

/// Reset the logger config
void logger_reset();

/// This is the default configuration procedure for the logger
/// If the file ""symap2ic_logger.conf" is found, it is used to configure the
/// logger and every other argument is ignored!
/// @level: Log level
/// @arg fname: Log to this file, if empty to stdout
/// @arg dual: If file is given, log also to stdout
void logger_default_config(log4cxx::LevelPtr level = log4cxx::Level::getWarn(),
		                   std::string fname = "",
		                   bool dual = false);

/// Load logger config from the given configuration file
/// @see ???
void logger_config_from_file(std::string filename);

/// adds a FileAppender to the given logger
log4cxx::AppenderPtr logger_append_to_file(
		std::string filename, log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger());

/// adds a ConsoleAppender to the given logger
log4cxx::AppenderPtr logger_append_to_cout(
		log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger());

/// Configure the logger to log everything above the given loglevel to a fiel
void logger_log_to_file(std::string filename, log4cxx::LevelPtr level);

/// Configure the logger to log everything above the given loglevel to stdout
void logger_log_to_cout(log4cxx::LevelPtr level);

/// Set the loglevel
void logger_set_loglevel(log4cxx::LoggerPtr l, log4cxx::LevelPtr level);

