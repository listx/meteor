#include "time.h"

/* Returns the time, with decisecond precision */
u64 get_time(void)
{
	struct timespec t;
	/* CLOCK_MONOTONIC_RAW requires Linux Kernel 2.6.28 or higher */
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	return (t.tv_sec * 100) + (t.tv_nsec/10000000);
}

void time_pretty(u64 time)
{
	float s;
	int h, m;
	s = ((float)time)/100;

	printf("%.2f", s);
	if (s < 60)
		printf("s\n");
	else {
		printf("s (");
		h = s/3600;
		m = s/60 - (h * 60);
		if (h)
			printf("%dh ", h);
		if (m)
			printf("%dm ", m);
		printf("%.2fs)\n", s - (m * 60) - (h * 3600));
	}
}
