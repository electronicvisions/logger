#include <stdio.h>
#include "logger_c.h"

int main()
{
	init(DEBUG2, "test_c.o", 0);
	toLog("test",3);

	return 0;
}
