#!/usr/bin/env python
import unittest
import tempfile
import shutil
import os.path
import pylogging as logger

class Test_Pylogging(unittest.TestCase):
    def setUp(self):
        logger.reset();
        self.temp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.temp)

    def test_cout_logging(self):
        logger.log_to_cout(logger.LogLevel.WARN)

        logger1 = logger.get("test");
        logger2 = logger.get("xyz");
        logger3 = logger.get("xyz.test");

        logger.set_loglevel(logger2, logger.LogLevel.DEBUG)
        logger.set_loglevel(logger3, logger.LogLevel.INFO)

        logger.LOG4CXX_FATAL(logger1, "FATAL")
        logger.LOG4CXX_ERROR(logger1, "ERROR")
        logger.LOG4CXX_WARN (logger1, "WARN ")
        logger.LOG4CXX_INFO (logger1, "INFO ")
        logger.LOG4CXX_DEBUG(logger1, "DEBUG")
        logger.LOG4CXX_TRACE(logger1, "TRACE")

        logger2.FATAL("FATAL")
        logger2.ERROR("ERROR")
        logger2.WARN ("WARN ")
        logger2.INFO ("INFO ")
        logger2.DEBUG("DEBUG")
        logger2.TRACE("TRACE")

        logger.LOG4CXX_FATAL(logger3, "FATAL")
        logger.LOG4CXX_ERROR(logger3, "ERROR")
        logger.LOG4CXX_WARN (logger3, "WARN ")
        logger.LOG4CXX_INFO (logger3, "INFO ")
        logger.LOG4CXX_DEBUG(logger3, "DEBUG")
        logger.LOG4CXX_TRACE(logger3, "TRACE")

    def test_file_logging(self):
        log = os.path.join(self.temp, 'test_file_logging.log')
        logger.log_to_file(log, logger.LogLevel.WARN)

        logger1 = logger.get("test");
        logger2 = logger.get("xyz");
        logger3 = logger.get("xyz.test");

        logger.set_loglevel(logger2, logger.LogLevel.DEBUG)
        logger.set_loglevel(logger3, logger.LogLevel.INFO)

        logger.LOG4CXX_FATAL(logger1, "FATAL")
        logger.LOG4CXX_ERROR(logger1, "ERROR")
        logger.LOG4CXX_WARN (logger1, "WARN")
        logger.LOG4CXX_INFO (logger1, "INFO")
        logger.LOG4CXX_DEBUG(logger1, "DEBUG")
        logger.LOG4CXX_TRACE(logger1, "TRACE")

        logger2.FATAL("FATAL")
        logger2.ERROR("ERROR")
        logger2.WARN ("WARN")
        logger2.INFO ("INFO")
        logger2.DEBUG("DEBUG")
        logger2.TRACE("TRACE")

        logger.LOG4CXX_FATAL(logger3, "FATAL")
        logger.LOG4CXX_ERROR(logger3, "ERROR")
        logger.LOG4CXX_WARN (logger3, "WARN")
        logger.LOG4CXX_INFO (logger3, "INFO")
        logger.LOG4CXX_DEBUG(logger3, "DEBUG")
        logger.LOG4CXX_TRACE(logger3, "TRACE")

        logger.reset()
        with open(log) as f:
            expected = \
"""FATAL test FATAL
ERROR test ERROR
WARN  test WARN
FATAL xyz FATAL
ERROR xyz ERROR
WARN  xyz WARN
INFO  xyz INFO
DEBUG xyz DEBUG
FATAL xyz.test FATAL
ERROR xyz.test ERROR
WARN  xyz.test WARN
INFO  xyz.test INFO
"""
            self.assertEqual(expected, f.read())

    def test_config_from_file(self):
        import inspect

        log = os.path.join(self.temp, 'log')
        config = os.path.join(self.temp, 'config')

        with open(config, 'w') as f:
            f.write("""
# Set root logger level to DEBUG and its only appender to A1.
log4j.rootLogger=WARN, A1

# More detail from xyz, but only a bit more from xyz.test
log4j.logger.xyz=TRACE
log4j.logger.xyz.test=INFO

# A1 is set to be a ConsoleAppender.
log4j.appender.A1=org.apache.log4j.FileAppender
log4j.appender.A1.File={log}
#log4j.appender.A1=org.apache.log4j.ConsoleAppender

# A1 uses PatternLayout.
log4j.appender.A1.layout=org.apache.log4j.ColorLayout
log4j.appender.A1.layout.Color=true
log4j.appender.A1.layout.PrintLocation=true
""".format(log=log))

        logger.config_from_file(config)

        logger1 = logger.get("test");
        logger2 = logger.get("xyz");
        logger3 = logger.get("xyz.test");

        filename = inspect.getframeinfo(inspect.currentframe())[0]
        loglines = []

        def logln(msg):
            f = inspect.stack()[1][0]
            l = inspect.getframeinfo(f)[1]
            loc = "  -> [33m" + filename + "[0m:[31m" + str(l) + "[0m"
            loglines.append(msg)
            loglines.append('\n')
            loglines.append(loc)
            loglines.append('\n')


        logger.LOG4CXX_FATAL(logger1, "FATAL"); logln("[31mFATAL [0mtest FATAL")
        logger.LOG4CXX_ERROR(logger1, "ERROR"); logln("[31mERROR [0mtest ERROR")
        logger.LOG4CXX_WARN (logger1, "WARN");  logln("[33mWARN  [0mtest WARN")
        logger.LOG4CXX_INFO (logger1, "INFO")
        logger.LOG4CXX_DEBUG(logger1, "DEBUG")
        logger.LOG4CXX_TRACE(logger1, "TRACE")

        logger2.FATAL("FATAL") ;logln("[31mFATAL [0mxyz FATAL")
        logger2.ERROR("ERROR") ;logln("[31mERROR [0mxyz ERROR")
        logger2.WARN ("WARN")  ;logln("[33mWARN  [0mxyz WARN")
        logger2.INFO ("INFO")  ;logln("[32mINFO  [0mxyz INFO")
        logger2.DEBUG("DEBUG") ;logln("[32mDEBUG [0mxyz DEBUG")
        logger2.TRACE("TRACE") ;logln("[32mTRACE [0mxyz TRACE")


        logger.LOG4CXX_FATAL(logger3, "FATAL") ;logln("[31mFATAL [0mxyz.test FATAL")
        logger.LOG4CXX_ERROR(logger3, "ERROR") ;logln("[31mERROR [0mxyz.test ERROR")
        logger.LOG4CXX_WARN (logger3, "WARN")  ;logln("[33mWARN  [0mxyz.test WARN")
        logger.LOG4CXX_INFO (logger3, "INFO")  ;logln("[32mINFO  [0mxyz.test INFO")
        logger.LOG4CXX_DEBUG(logger3, "DEBUG")
        logger.LOG4CXX_TRACE(logger3, "TRACE")

        logger.reset() # Hope this flushes the logger ;)
        with open(log) as f:
            expected = "".join(loglines)
            self.assertEqual(expected, f.read())

if __name__ == '__main__':
    unittest.main()
