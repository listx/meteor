#include "io.h"

/* Error messages */
void error_msg(const char *str, ...)
{
	va_list arg;
	va_start(arg, str);
	vfprintf(stderr, str, arg);
	va_end(arg);
	fprintf(stderr, "\n");
}

void error(const char *str, ...)
{
	fprintf(stderr, "error: ");
	error_msg(str);
}

void fatal(const char *str, ...)
{
	fprintf(stderr, "fatal: ");
	error_msg(str);
	exit(1);
}
