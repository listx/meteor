#ifndef IO_H
#define IO_H

#include <assert.h>	/* assert() */
#include <stdio.h>	/* printf() */
#include <stdlib.h>	/* malloc(), exit(), atoi() */
#include <getopt.h>	/* getopt_long() */
#include <stdarg.h>	/* va_list, va_start(), etc. */
#include <string.h>	/* strtok(), strlen(), strcat(), etc. */

extern void error(const char *str, ...);
extern void fatal(const char *str, ...);

#endif
