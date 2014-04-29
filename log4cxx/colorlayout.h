#pragma once

#include <sstream>
#include <iomanip>
#include <log4cxx/layout.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/datelayout.h>

namespace log4cxx {

struct ColorLayout : public helpers::DateLayout
{
	DECLARE_LOG4CXX_OBJECT(ColorLayout)

	//
	// @arg date_format: values are: NULL, RELATIVE, ABSOLUTE, DATE, ISO8601
	ColorLayout(bool use_color=true, LogString date_format="NULL");

	void setColor(bool value);
	void setPrintLocation(bool value);
	void setDateFormat(const LogString& dateFormat1);

	virtual void format(LogString& output,
						spi::LoggingEventPtr const& event,
						log4cxx::helpers::Pool& pool) const;

	virtual bool ignoresThrowable() const { return true; }
	virtual void setOption(const LogString& /* option */,
						   const LogString& /* value */);

private:
	bool mColorless;
	bool mPrintLocation;
};

LOG4CXX_PTR_DEF(ColorLayout);

} // namespace log4cxx
