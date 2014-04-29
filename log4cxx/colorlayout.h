#pragma once

#include <sstream>
#include <iomanip>
#include <log4cxx/layout.h>
#include <log4cxx/spi/loggingevent.h>

namespace log4cxx {

struct ColorLayout : public Layout
{
	DECLARE_LOG4CXX_OBJECT(ColorLayout)

	ColorLayout(bool use_color=true, bool print_time=true, bool relative_time=false);

	void setColor(bool value);
	void setPrintLocation(bool value);
	void setPrintTime(bool value);
	void setRelativeTime(bool value);

	virtual void format(LogString& output,
						spi::LoggingEventPtr const& event,
						log4cxx::helpers::Pool& pool) const;

	virtual bool ignoresThrowable() const { return true; }
	virtual void activateOptions(log4cxx::helpers::Pool&) {}
	virtual void setOption(const LogString& /* option */,
						   const LogString& /* value */);

private:
	bool mColorless;
	bool mPrintLocation;
	bool mPrintTime;
	bool mRelativeTime;
};

LOG4CXX_PTR_DEF(ColorLayout);

} // namespace log4cxx
