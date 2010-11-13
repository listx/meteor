#ifndef TEST_H
#define TEST_H

#include "io.h"
#include "types.h"
#include "movegen.h"
#include "time.h"

extern void test_perft(const char *sfen, int plydepth);
extern void test_perft_display(struct position *pos, int plydepth);
extern u64 perft(struct position *pos, int plydepth);

#endif
