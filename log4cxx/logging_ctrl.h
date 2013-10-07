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

/// Load logger config from the given configuration file
/// @see ???
void logger_config_from_file(std::string filename);

/// Configure the logger to log everything above the given loglevel to a fiel
void logger_log_to_file(std::string filename, log4cxx::LevelPtr level);

/// Configure the logger to log everything above the given loglevel to stdout
void logger_log_to_cout(log4cxx::LevelPtr level);

/// Set the loglevel
void logger_set_loglevel(log4cxx::LoggerPtr l, log4cxx::LevelPtr level);

