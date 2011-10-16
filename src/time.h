#ifndef TIME_H
#define TIME_H

#include <time.h>
#include "io.h"
#include "types.h"	/* u64 */

extern u64 get_time(void);
extern void time_pretty(u64 time);

#endif
