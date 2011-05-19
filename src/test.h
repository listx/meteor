#ifndef TEST_H
#define TEST_H

#include "hash.h"
#include "io.h"
#include "movegen.h"
#include "random.h"
#include "thread.h"
#include "time.h"
#include "types.h"

extern int test_zob;
extern int hashMB;

extern void test_perft(const char *sfen, int plydepth, int threads);
extern void test_perft_display(struct position *pos, int plydepth, int threads);
extern u64 perft(struct position *pos, int plydepth);
extern u64 perft_zob(struct position *pos, int plydepth);
extern u64 perft_hash(struct position *pos, int plydepth, struct tt_perft *tt);

extern void move_show_line(struct position *pos, struct move *mlist, int i, char *sfen, int sfen_maxlen, int fmn);
extern void *start_routine(void *thread_id);
extern void idle_work_loop(int *thread_id);
extern void kill_thread_vars();
extern void init_thread_vars();
extern void init_threads(int threads);

extern void randk_verify();

#endif
