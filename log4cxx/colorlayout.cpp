#include "colorlayout.h"

#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/relativetimedateformat.h>
#include <log4cxx/helpers/absolutetimedateformat.h>

using namespace log4cxx::helpers;

namespace log4cxx {

ColorLayout::ColorLayout(bool use_color, LogString date_format) :
	helpers::DateLayout(date_format),
	mColorless(!use_color),
	mPrintLocation(false)
{}

void ColorLayout::setColor(bool value)
{
	mColorless = !value;
}

void ColorLayout::setPrintLocation(bool value)
{
	mPrintLocation = value;
}

void ColorLayout::format(LogString& _output,
						 spi::LoggingEventPtr const& event,
						 log4cxx::helpers::Pool& p) const
{
	using namespace std;

	ostringstream output;

#ifdef CONFIG_NO_COLOR
	const bool colorless = true;
#else
	const bool colorless = mColorless;
#endif

	enum { Black, Red, Green,
		Yellow, Blue, Purple,
		Marine, Gray, Reset };

	static const char * const color[9] = {
		"\33[30m", "\33[31m", "\33[32m",
		"\33[33m", "\33[34m", "\33[35m",
		"\33[36m", "\33[37m", "\33[0m" };

	int const level = event->getLevel()->toInt();
	if (colorless) {
		// no color
	} else if (level >= Level::ERROR_INT) {
		output << color[Red];
	} else if (level >= Level::WARN_INT) {
		output << color[Yellow];
	} else {
		output << color[Green];
	}

	LogString date;
	formatDate(date, event, p);
	if (!date.empty())
		date += " ";
	//formatDate(date, event->getTimeStamp(), p);

	output
		<< setw(6) << left << event->getLevel()->toString()
		<< (colorless ? "" : color[Reset])
		<< date
		<< event->getLoggerName() << " "
		<< event->getRenderedMessage();
	if (mPrintLocation) {
		if (colorless)
			output << "\n  -> "
				   << event->getLocationInformation().getFileName()
				   << ":"
				   << event->getLocationInformation().getLineNumber();
		else
			output << "\n  -> "
				   << color[Yellow] << event->getLocationInformation().getFileName()
				   << color[Reset]  << ":"
				   << color[Red]    << event->getLocationInformation().getLineNumber()
				   << color[Reset];
	}
	output << endl;
	_output = output.str();
}

void ColorLayout::setOption(const LogString& option,  const LogString& value)
{
	helpers::DateLayout::setOption(option, value);
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("COLOR"), LOG4CXX_STR("color")))
	{
		setColor(OptionConverter::toBoolean(value, false));
	}
	if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PRINTLOCATION"), LOG4CXX_STR("printlocation")))
	{
		setPrintLocation(OptionConverter::toBoolean(value, false));
	}
}


} // namespace log4cxx

using namespace log4cxx;
IMPLEMENT_LOG4CXX_OBJECT(ColorLayout)
