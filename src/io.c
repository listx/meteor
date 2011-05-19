#include "io.h"

/* Error messages */
void error(const char *str, ...)
{
	va_list arg;
	fprintf(stderr, "error: ");
	va_start(arg, str);
	vfprintf(stderr, str, arg);
	va_end(arg);
	fprintf(stderr, "\n");
}

void fatal(const char *str, ...)
{
	va_list arg;
	fprintf(stderr, "fatal: ");
	va_start(arg, str);
	vfprintf(stderr, str, arg);
	va_end(arg);
	fprintf(stderr, "\n");
	exit(1);
}

void dbg(const char *str, ...)
{
	va_list arg;
#ifdef DEBUG
	printf("debug: ");
	va_start(arg, str);
	vfprintf(stdout, str, arg);
	va_end(arg);
	fprintf(stdout, "\n");
#else
	/* don't do anything (visible) with str; to silence compilation warnings
	 * (whenever we're not building a debug build) about not using str, we
	 * use it here in a useless way
	 */
	va_start(arg, str);
	va_end(arg);
#endif
}
