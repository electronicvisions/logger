#include <syslog.h>

#include <sstream>
#include <string>
#include <utility>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/variadic/size.hpp>


namespace logger {

enum class LogPriority : int
{
	DEBUG = LOG_DEBUG,
	INFO = LOG_INFO,
	WARN = LOG_WARNING,
	ERROR = LOG_ERR,
	CRIT = LOG_CRIT,
	NONE = -1
};

#ifdef SYSLOG_THRESHOLD
constexpr LogPriority prio_syslog_threshold = SYSLOG_THRESHOLD;
#else
constexpr LogPriority prio_syslog_threshold = LogPriority::INFO;
#endif

namespace detail {

/**
 * Return string that reflects given logging priority.
 * @param prio Logging priority.
 * @return String of logging priority.
 */
constexpr char const* prio_to_string(LogPriority prio)
{
	switch (prio) {
		case LogPriority::DEBUG:
			return "DEBUG";
		case LogPriority::INFO:
			return "INFO";
		case LogPriority::WARN:
			return "WARN";
		case LogPriority::ERROR:
			return "ERROR";
		case LogPriority::CRIT:
			return "CRITICAL";
		default:
			return "NONE";
	}
}

} // namespace detail

/**
 * Syslog given log messages.
 *
 * Concatenates multiple arguments to one message.
 * @param args Log message. Can be [single, multiple] [string, stringstream].
 */
template <logger::LogPriority Prio, typename... Args>
void logger_syslog(Args&&... args)
{
	if constexpr (Prio <= prio_syslog_threshold && Prio != logger::LogPriority::NONE) {
		std::stringstream msg;
		(msg << ... << std::forward<Args>(args));

		std::string msg_string = msg.str();
		syslog(
		    static_cast<int>(Prio), "[%s] %s", logger::detail::prio_to_string(Prio),
		    msg_string.c_str());
	}
}

/**
 * Macro for syslogging a message.
 *
 * Requires opening syslog beforehand and closing afterwards.
 * @param PRIO Log priority.
 * @param message Log message, which is streamed into a sstream.
 */
#define LOGGER_SYSLOG_STREAM(PRIO, message)                                                        \
	do {                                                                                           \
		if constexpr (                                                                             \
		    logger::LogPriority::PRIO <= logger::prio_syslog_threshold &&                          \
		    logger::LogPriority::PRIO != logger::LogPriority::NONE) {                              \
			std::stringstream msg;                                                                 \
			msg << message;                                                                        \
                                                                                                   \
			std::string msg_string = msg.str();                                                    \
			syslog(                                                                                \
			    static_cast<int>(logger::LogPriority::PRIO), "[%s] %s",                            \
			    logger::detail::prio_to_string(logger::LogPriority::PRIO), msg_string.c_str());    \
		}                                                                                          \
	} while (0)

/**
 * Macro for syslogging a message.
 *
 * Requires opening syslog beforehand and closing afterwards.
 * @param PRIO Log priority.
 * @param __VA_ARGS__ Log message. fprint-style arguments are expected.
 */
#define LOGGER_SYSLOG_FORMAT(PRIO, ...)                                                            \
	logger::logger_syslog<logger::LogPriority::PRIO>(__VA_ARGS__);

/**
 * Macro for syslogging a message.
 *
 * Requires opening syslog beforehand and closing afterwards.
 * @param PRIO Log priority.
 * @param __VA_ARGS__ Log message. If one argument is given, it is streamed into a sstream.
 * Otherwise, fprint-style arguments are expected.
 */
#define LOGGER_SYSLOG(PRIO, ...)                                                                   \
	BOOST_PP_IF(                                                                                   \
	    BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 1), LOGGER_SYSLOG_STREAM,              \
	    LOGGER_SYSLOG_FORMAT)                                                                      \
	(PRIO, __VA_ARGS__)

/**
 * Open syslog.
 *
 * @param ident Identifier for the syslog. This should be changed e.g. the repo name where the
 * logger is used.
 * @param option Syslog option given as bitmask flags. Can be combined via | operator.
 * @param facility Syslog facility.
 */
inline void syslog_open(
    const char* ident = "default", int option = LOG_CONS, int facility = LOG_USER)
{
	if constexpr (prio_syslog_threshold != logger::LogPriority::NONE) {
		openlog(ident, option, facility);
	}
}

/**
 * Close syslog.
 */
inline void syslog_close()
{
	if constexpr (prio_syslog_threshold != logger::LogPriority::NONE) {
		closelog();
	}
}

/**
 * Macro suitable for one time use of logging.
 *
 * Open syslog, log the given message and close syslog again.
 * This macro is not suitable for low loglevels since opening and closing syslog multiple times adds
 * unecessary overhead.
 * @param prio Log priority
 * @param ident Identifier for the service.
 * @param message Log message.
 */
#define LOGGER_OPEN_SYSLOG_CLOSE(prio, ident, ...)                                                 \
	do {                                                                                           \
		logger::syslog_open(ident);                                                                \
		LOGGER_SYSLOG(prio, __VA_ARGS__);                                                          \
		logger::syslog_close();                                                                    \
	} while (0);

} // namespace logger
