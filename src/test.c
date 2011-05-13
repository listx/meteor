#include "test.h"

pthread_mutex_t lock;
pthread_cond_t wake;
pthread_cond_t work_done;
int *thread_id;
int moves_initial;
char sfen[100] = {'\0'};
int sfen_maxlen;
int fmn;

struct work_unit {
	struct position pos;
	struct move m;
	int plydepth;
	u64 nodes;
	bool in_progress;
	bool complete;
};

struct work_unit *wunit;

void test_perft(const char *str, int plydepth, int threads)
{
	struct position pos;
	import_sfen(str, &pos);
	disp_pos(&pos);
	test_perft_display(&pos, plydepth, threads);
}

void test_perft_display(struct position *pos, int plydepth, int threads)
{
	struct move mlist[256];
	u32 undo_info;
	int color, i;
	u64 subnodes, nodes, t1, t2, t3;
	nodes = 0;
	subnodes = 0;
	fmn = FULL_MOVE_NUMBER;
	color = pos->info & BIT_TURN;
	moves_initial = movegen(pos, mlist, color);

	/* look at all the possible sfen's that result, and determine the
	 * longest one (so that we can format our output better below
	 */
	sfen_maxlen = 0;
	for (i = 0; i < moves_initial; i++, pos->checkers = 0x0ULL) {
		move_do(pos, &mlist[i].info, &undo_info);
		if (our_color(&pos->info) == W)
			export_sfen(pos, sfen, fmn + 1);
		else
			export_sfen(pos, sfen, fmn);
		if (strlen(sfen) > (unsigned)sfen_maxlen)
			sfen_maxlen = strlen(sfen);
		move_undo(pos, &mlist[i].info, &undo_info);
	}

	printf("\nCounting all nodes to plydepth %d\n", plydepth);
	printf("Threads: %d\n", threads);
	printf("Found %d moves from initial position\n\n", moves_initial);
	if (!moves_initial) {
		printf("%s is ", disp_color(&pos->info));
		printf("%s!\n", (pos->checkers ? "checkmated" : "stalemated"));
		return;
	}
	printf("no. move pmv pcp ppr mvtyp chk sfen of child node (plydepth %d)\n", plydepth - 1);
        printf("--- ---- --- --- --- ----- --- -------------------------------------------------\n");

	t1 = get_time(); /* record time */
	if (threads == 1) {
		/*
		 * If plydepth > 1, then try out each move, recursively, for
		 * until depth is 0 (or checkmate) --- and report back the total
		 * nodecount for each move
		 */
		for (i = 0; i < moves_initial; i++, pos->checkers = 0x0ULL) {
			move_do(pos, &mlist[i].info, &undo_info);
			move_show_line(pos, &mlist[i], i, sfen, sfen_maxlen, fmn);

			/* Recurse into this move, up to plydepth times. */
			subnodes = perft(pos, plydepth - 1);

			printf("%"PRIu64"\n", subnodes);
			move_undo(pos, &mlist[i].info, &undo_info);
			nodes += subnodes;
		}
	} else {
		/* If multithreaded, then the order of execution is
		 * nondeterministic; the best we can do is "print" the results
		 * of each move's recursive subnodes and everything into a file,
		 * then sort the file based on move number and display that one.
		 * The "cool" way to do it would be to print a predetermined
		 * number of newlines and then just "fill in" each move's
		 * results as they are calculated by the threads. E.g., if move
		 * 14 is done, then go up (\e[A) MOVES - 14 lines, then print
		 * the line, then come back down (\e[B) MOVES - 14 lines ---
		 * repeat for every moveline.
		 */

		/* initialize mutex locks and condition variables */
		init_thread_vars();

		/* set up work units */
		wunit = malloc(sizeof(*wunit) * moves_initial);
		for (i = 0; i < moves_initial; i++) {
			memcpy(&wunit[i].pos, pos, sizeof(*pos));
			memcpy(&wunit[i].m, &mlist[i], sizeof(&mlist[i]));
			move_do(&wunit[i].pos, &mlist[i].info, &undo_info);
			wunit[i].plydepth = plydepth - 1;
			wunit[i].nodes = 0x0ULL;
			wunit[i].in_progress = false;
			wunit[i].complete = false;
		}

		/* spawn worker threads */
		pthread_mutex_lock(&lock);
		init_threads(threads);
		pthread_mutex_unlock(&lock);

		/* now enter a loop; every time a wunit is complete, we check if
		 * it's the last one completed; if so, we break out of this
		 * loop; this way, we effectively wait until all work has been
		 * completed.
		 */
		for (;;) {
			pthread_mutex_lock(&lock);
			pthread_cond_wait(&work_done, &lock);

			for (i = 0; i < moves_initial; i++) {
				if (!wunit[i].complete)
					break;
			}
			if (i == moves_initial) {
				pthread_mutex_unlock(&lock);
				break;
			}
			pthread_mutex_unlock(&lock);
		}
	}

	t2 = get_time();
	t3 = t2 - t1;

	if (threads > 1) {
		/* now that all work is done, display the results */
		printf("\nSorted:\n");
		for (i = 0; i < moves_initial; i++, pos->checkers = 0x0ULL) {
			move_show_line(&wunit[i].pos, &mlist[i], i, sfen, sfen_maxlen, fmn);
			printf("%"PRIu64"\n", wunit[i].nodes);
			nodes += wunit[i].nodes;
		}
	}

	printf("\nNodes: %"PRIu64"\n", nodes);
	if (t3) {
		printf("Time: ");
		time_pretty(t3);
		printf("Knps: %"PRIu64"\n", nodes/t3/10);
	}
	printf("\n");

	/* release resources */
	free(wunit);
	free(thread_id);
	kill_thread_vars();
}

void move_show_line(struct position *pos, struct move *mlist, int i, char *sfen, int sfen_maxlen, int fmn)
{
	int j;

	if (i + 1 < 100)
		printf("0");
	if (i + 1 < 10)
		printf("0");
	printf("%d ", i + 1);
	disp_move(&mlist->info);
	find_checkers(pos);
	if (pos->checkers)
		printf(" +  ");
	else
		printf("    ");
	if (our_color(&pos->info) == W)
		export_sfen(pos, sfen, fmn + 1);
	else
		export_sfen(pos, sfen, fmn);
	printf("%s", sfen);

	/* add padding */
	for (j = strlen(sfen); j < sfen_maxlen; j++)
		printf(" ");
	printf(" => ");
}

u64 perft(struct position *pos, int plydepth)
{
	u64 nodes = 0x0ULL;
	int moves, i;
	struct move mlist[256];
	u32 undo_info;
        moves = movegen(pos, mlist, our_color(&pos->info));
        if (plydepth > 1) {
                for (i = 0; i < moves; i++) {
			move_do(pos, &mlist[i].info, &undo_info);
                        nodes += perft(pos, plydepth - 1);
			move_undo(pos, &mlist[i].info, &undo_info);
                }
        } else {
                return moves;
        }
        return nodes;
}

/* Initialize mutexes and condition variables */
void init_thread_vars()
{
	int ok;

	ok = pthread_mutex_init(&lock, NULL);
	if (ok != 0)
		fatal("pthread_mutex_init() failed");

	ok = pthread_cond_init(&wake, NULL);
	if (ok != 0)
		fatal("pthread_cond_init() failed");

	ok = pthread_cond_init(&work_done, NULL);
	if (ok != 0)
		fatal("pthread_cond_init() failed");
}

void kill_thread_vars()
{
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&wake);
	pthread_cond_destroy(&work_done);
}

void *start_routine(void *thread_id)
{
	idle_work_loop((int*)thread_id);
	return NULL;
}

/* Idle as long as there is no work; if there is work, then do it and resume the
 * idle loop */
void idle_work_loop(int *thread_id)
{
	int i;

	for (;;) {
		/* let's try to do the work now */
		pthread_mutex_lock(&lock);
		dbg("thread %d owns &lock", *thread_id);
		/* find suitable wunit */
		for (i = 0; i < moves_initial; i++) {
			if (wunit[i].in_progress == false) {
				dbg("thread %d found work!", *thread_id);
				wunit[i].in_progress = true;
				break;
			}
		}
		if (i == moves_initial) {
			/* there are no more work units that have not been
			 * taken; most likely, there are some peer worker
			 * threads that are still working away at their job;
			 * let's just exit out of this infinite loop for now...
			 */
			pthread_mutex_unlock(&lock);
			break;
		}
		pthread_mutex_unlock(&lock);

		/* since wunit[i].pos and wunit[i].nodes is ONLY accessed by
		 * this thread, we can leave it outside of the mutex; this way,
		 * other worker threads don't have to wait until this (possibly
		 * very long) perft() call is finished
		 */
		wunit[i].nodes = perft(&wunit[i].pos, wunit[i].plydepth);

		pthread_mutex_lock(&lock);
		wunit[i].complete = true;
		dbg("thread %d finished a work unit", *thread_id);

		move_show_line(&wunit[i].pos, &wunit[i].m, i, sfen, sfen_maxlen, fmn);
		printf("%"PRIu64"\n", wunit[i].nodes);

		pthread_cond_signal(&work_done);
		pthread_mutex_unlock(&lock);
	}
}

/* Wake up worker threads and put them in an idle loop; we use a condition
 * variable to wake them up and put them to work.
 */
void init_threads(int threads)
{
	int i;
	thread_id = malloc(sizeof(*thread_id) * threads);

	for (i = 0; i < threads; i++) {
		thread_id[i] = i;
		/* Create a new pthread_t variable for this thread */
		static pthread_t pthread_id;
		/* Create the thread */
		pthread_create(&pthread_id, NULL, start_routine, (void*)&thread_id[i]);
		dbg("thread %d created\n", thread_id[i]);

		/* Detach the thread because we don't care about the thread's
		 * exit status */
		pthread_detach(pthread_id);
	}
}

void randk_verify()
{
	int i;
	u64 x, n1, n2;
	n1 = 13596816608992115578ULL;
	n2 = 5033346742750153761ULL;

	printf("Verifying the PRNG...\n");

	/* Generate 10^9 B64MWC()s */
	for (i = 0; i < 1000000000; i++)
		x = B64MWC();
	if (x == n1) {
		printf("Test 1: B64MWC(): expected %"PRIu64", got %"PRIu64"\n", n1, x);
		printf("Test 1: OK\n");
	} else {
		error("Test 1: B64MWC(): expected %"PRIu64", got %"PRIu64"", n1, x);
		fatal("Test 1: FAIL");
	}

	/* Generate 10^9 KISSes: */
	for (i = 0; i < 1000000000; i++)
		x = randk();
	if (x == n2) {
		printf("Test 2: randk(): expected %"PRIu64", got %"PRIu64"\n", n2, x);
		printf("Test 2: OK\n");
	} else {
		error("Test 2: randk(): expected %"PRIu64", got %"PRIu64"", n2, x);
		fatal("Test 2: FAIL");
	}
}
