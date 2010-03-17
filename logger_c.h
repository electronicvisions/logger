#ifndef __LOGGER_C_H__
#define __LOGGER_C_H__

enum LEVELS {
	ERROR,
	WARNING,
	INFO,
	DEBUG0,
	DEBUG1,
	DEBUG2
};

#ifdef __cplusplus
extern "C"
#endif
void log_init( unsigned int level, const char* filename, int dual );

#ifdef __cplusplus
extern "C"
#endif
void log_print(unsigned int level, const char* msg);

#endif // __LOGGER_C_H__
