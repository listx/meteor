#include "io.h"

/* Error messages */
void error(const char *str, ...)
{
	fprintf(stderr, "error: ");
	va_list arg;
	va_start(arg, str);
	vfprintf(stderr, str, arg);
	va_end(arg);
	fprintf(stderr, "\n");
}

void fatal(const char *str, ...)
{
	fprintf(stderr, "fatal: ");
	va_list arg;
	va_start(arg, str);
	vfprintf(stderr, str, arg);
	va_end(arg);
	fprintf(stderr, "\n");
	exit(1);
}
