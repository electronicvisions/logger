#include <stdio.h>
#include "logger_c.h"

int main()
{
	log_init(DEBUG2, "test_c-logfile.txt", /* dual log */1);
	log_print(3,"test3");
	log_print(2,"test2");
	log_print(1,"test1");
	log_print(0,"test0");

	return 0;
}
