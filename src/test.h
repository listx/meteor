#ifndef TEST_H
#define TEST_H

#include "io.h"
#include "types.h"
#include "movegen.h"
#include "time.h"
#include "thread.h"

extern void test_perft(const char *sfen, int plydepth, int threads);
extern void test_perft_display(struct position *pos, int plydepth, int threads);
extern u64 perft(struct position *pos, int plydepth);

extern void move_show_line(struct position *pos, struct move *mlist, int i, char *sfen, int sfen_maxlen, int fmn);
extern void *start_routine(void *thread_id);
extern void idle_work_loop(int *thread_id);
extern void kill_thread_vars();
extern void init_thread_vars();
extern void init_threads(int *thread_id, int threads);

#endif
