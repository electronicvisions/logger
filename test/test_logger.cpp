
#include <gtest/gtest.h>

#include "logger.h"

class LoggerTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
	    log4cxx::BasicConfigurator::resetConfiguration();
	}

	virtual void TearDown()
	{
	}
};

TEST_F(LoggerTest, TestThrowActive)
{
	log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger("loggertests.throw");
	logger_write_to_cout(logger);

	ASSERT_THROW(LOG4CXX_FATAL(logger, "FATAL_MSG"), std::runtime_error);

	ASSERT_NO_THROW(LOG4CXX_ERROR(logger, "ERROR_MSG"));
	ASSERT_NO_THROW(LOG4CXX_WARN(logger, "WARN_MSG "));
	ASSERT_NO_THROW(LOG4CXX_INFO(logger, "INFO_MSG "));
	ASSERT_NO_THROW(LOG4CXX_DEBUG(logger, "DEBUG_MSG"));
	ASSERT_NO_THROW(LOG4CXX_TRACE(logger, "TRACE_MSG"));
}
