#!/usr/bin/env python
from builtins import str
import pylogging

def test_msg(print_location, date_format):
    pylogging.reset()
    layout = pylogging.ColorLayout(True, date_format)
    layout.setOption("printlocation", str(print_location).lower())
    layout.activateOptions()
    appender = pylogging.ConsoleAppender(layout)
    appender.setOption("target", pylogging.ConsoleAppender.getSystemErr())
    appender.activateOptions()
    l = pylogging.get_root()
    pylogging.set_loglevel(l, pylogging.LogLevel.DEBUG)
    l.addAppender(appender)
    l = pylogging.get("manual_config")
    l.warn("time format: {}".format(date_format))

def default_test_msg(print_location, date_format):
    pylogging.reset()
    pylogging.default_config(
            level=pylogging.LogLevel.DEBUG,
            print_location=print_location,
            date_format=date_format,
            color=True)
    l = pylogging.get("default_config")
    l.debug("time format: {}".format(date_format))

for print_location in (True, False):
    for date_format in ('NULL', 'RELATIVE', 'ABSOLUTE', 'DATE', 'ISO8601'):
        test_msg(print_location, date_format)
        default_test_msg(print_location, date_format)
