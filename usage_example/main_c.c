#include <stdio.h>
#include "logger_c.h"

int main()
{
	init(DEBUG2, "test_c-logfile.txt", /* dual log */1);
	toLog(3,"test3");
	toLog(2,"test2");
	toLog(1,"test1");
	toLog(0,"test0");

	return 0;
}
