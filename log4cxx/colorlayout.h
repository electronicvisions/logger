#pragma once

#include <sstream>
#include <iomanip>
#include <log4cxx/layout.h>
#include <log4cxx/spi/loggingevent.h>

namespace log4cxx {

struct ColorLayout : public Layout
{
	ColorLayout(bool use_color=true) :
		mColorless(!use_color)
	{}

	virtual void format(LogString& output,
						spi::LoggingEventPtr const& event,
						log4cxx::helpers::Pool& pool) const;

	virtual bool ignoresThrowable() const { return true; }
	virtual void activateOptions(log4cxx::helpers::Pool&) {}
	virtual void setOption(const LogString& /* option */,
						   const LogString& /* value */) {}

private:
	bool mColorless;
};


inline
void ColorLayout::format(LogString& _output,
						 spi::LoggingEventPtr const& event,
						 log4cxx::helpers::Pool&) const
{
	using namespace std;

	enum { Black, Red, Green,
		Yellow, Blue, Purple,
		Marine, Gray, Reset };

	static const char * const color[9] = {
		"\33[30m", "\33[31m", "\33[32m",
		"\33[33m", "\33[34m", "\33[35m",
		"\33[36m", "\33[37m", "\33[0m" };

	ostringstream output;
	int const level = event->getLevel()->toInt();
	if (mColorless) {
		// no color
	} else if (level >= Level::ERROR_INT) {
		output << color[Red];
	} else if (level >= Level::WARN_INT) {
		output << color[Yellow];
	} else {
		output << color[Green];
	}

	output
		<< setw(6) << left << event->getLevel()->toString()
		<< (mColorless ? "" : color[Reset])
		<< event->getLoggerName() << " "
		<< event->getRenderedMessage()
		<< endl;
	_output = output.str();
}

} // namespace log4cxx
