#include "test.h"

pthread_mutex_t lock;
pthread_cond_t wake;
pthread_cond_t work_done;
pthread_t *pthread_id;
int *thread_id;
int moves_initial;
char sfen[100] = {'\0'};
int sfen_maxlen;
int fmn;
u64 perft_hits = 0;
u64 perft_queries = 0;
int test_zob;
int hashMB;

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
	u64 subnodes, nodes, zkey, t1, t2, t3;
	nodes = 0;
	subnodes = 0;
	fmn = FULL_MOVE_NUMBER;
	color = pos->info & BIT_TURN;
	moves_initial = movegen(pos, mlist, color);

	if (hashMB && !test_zob) {
		printf("\nInitializing hashtable...");
		init_tt_perft(hashMB);
		printf("OK (%"PRIu32" bytes (%"PRIu32"MB), %u entries (%u buckets))\n", tt_hash_size(), (tt_hash_size() >> 20), tt.entries, tt.entries * 4);
	} else {
		printf("\nHashtable disabled\n");
	}

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
	if (test_zob)
		printf("Testing zobrist key mechanism\n");
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
			zkey = pos->zkey;
			move_do(pos, &mlist[i].info, &undo_info);
			move_show_line(pos, &mlist[i], i, sfen, sfen_maxlen, fmn);

			/* Recurse into this move, up to plydepth times. */
			if (test_zob) {
				subnodes = perft_zob(pos, plydepth - 1);
			} else {
				if (hashMB)
					subnodes = perft_hash(pos, plydepth - 1);
				else
					subnodes = perft(pos, plydepth - 1);
			}

			printf("%"PRIu64"\n", subnodes);
			move_undo(pos, &mlist[i].info, &undo_info);
			if (zkey != pos->zkey) {
				error("zkey mismatch\n");
				disp_pos(pos);
				assert(0);
			}
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
			memcpy(&wunit[i].m, &mlist[i], sizeof(mlist[i]));
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

		/* now enter a loop; if a wunit is complete, we check if it's
		 * the last one completed; if so, we break out of this loop;
		 * this way, we effectively wait until all work has been
		 * completed.
		 *
		 * NOTE: technically, the worker threads will contend for the
		 * &lock mutex much more, which means that this infinite loops
		 * will only really get exposure once or twice, even if there
		 * are 50+ work units.
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
		/* wait until all threads have exited completely (there is a
		 * tiny gap between a thread completing its work, and actually
		 * breaking out of its work loop and exiting cleanly; we wait
		 * for that gap here)
		 */

		for (i = 0; i < threads; i++) {
			pthread_join(pthread_id[i], NULL);
			dbg("thread %d exited successfully\n", thread_id[i]);
		}

		/* now that all work is done, display the results */
		printf("\nSorted:\n");
		for (i = 0; i < moves_initial; i++, pos->checkers = 0x0ULL) {
			move_show_line(&wunit[i].pos, &mlist[i], i, sfen, sfen_maxlen, fmn);
			printf("%"PRIu64"\n", wunit[i].nodes);
			nodes += wunit[i].nodes;
		}

		/* release resources */
		kill_thread_vars();
		free(wunit);
		free(thread_id);
		free(pthread_id);
	}

	printf("\nNodes: %"PRIu64"\n", nodes);
	if (t3) {
		printf("Time: ");
		time_pretty(t3);
		printf("Knps: %"PRIu64"\n", nodes/t3/10);
	}
	if (hashMB && !test_zob) {
		printf("Hash access: ");
		if (perft_queries)
			printf("%.2f%%", (long)perft_hits/(double)perft_queries * 100);
			printf(" (%"PRIu64" queries, %"PRIu64" hits, %"PRIu64" writes)\n", perft_queries, perft_hits, tt.writes);
		free_tt();
	}
	if (test_zob) {
		printf("Zobrist key mechanism OK\n");
	}
	printf("\n");
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

u64 perft_zob(struct position *pos, int plydepth)
{
	u64 nodes = 0x0ULL;
	int moves, i;
	struct move mlist[256];
	u32 undo_info;
	u64 zkey;
        moves = movegen(pos, mlist, our_color(&pos->info));
        if (plydepth > 1) {
                for (i = 0; i < moves; i++) {
			zkey = pos->zkey;
			move_do(pos, &mlist[i].info, &undo_info);
                        nodes += perft(pos, plydepth - 1);
			move_undo(pos, &mlist[i].info, &undo_info);
			if (zkey != pos->zkey) {
				assert(0);
			}
                }
        } else {
                return moves;
        }
        return nodes;
}

u64 perft_hash(struct position *pos, int plydepth)
{
	u64 nodes = 0x0ULL;
	int moves, i;
	struct move mlist[256];
	u32 undo_info;
	struct tt_perft_bucket *bucket;
	u64 d = plydepth;

	/* Query the hashtable for an existing bucket under the zkey entry */
	bucket = get_bucket_lockless(&pos->zkey, &d);
	perft_queries++;

	/* If a bucket is found, return the value from the stored variale */
	if (bucket != NULL) {
		perft_hits++;
		return get_nodes(&bucket->info);
	}

        moves = movegen(pos, mlist, our_color(&pos->info));
        if (plydepth > 1) {
                for (i = 0; i < moves; i++) {
			move_do(pos, &mlist[i].info, &undo_info);
                        nodes += perft_hash(pos, plydepth - 1);
			move_undo(pos, &mlist[i].info, &undo_info);
                }

		/* Store calculated data into hashtable */
		update_entry(&pos->zkey, &nodes, &plydepth);
                return nodes;
        }

	/* Store the number of nodes for depth 1, so that we don't have to run
	 * the expensive movegen() function the next time we call this function
	 * again for depth 1
	 */
	int depth1 = 1;
	u64 m = moves;
	update_entry(&pos->zkey, &m, &depth1);
	return moves;
}

/* Initialize mutexes and condition variables */
void init_thread_vars(void)
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

void kill_thread_vars(void)
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
		pthread_mutex_lock(&lock);
		if (hashMB) {
			pthread_mutex_unlock(&lock);
			wunit[i].nodes = perft_hash(&wunit[i].pos, wunit[i].plydepth);
		} else {
			if (test_zob) {
				pthread_mutex_unlock(&lock);
				wunit[i].nodes = perft_zob(&wunit[i].pos, wunit[i].plydepth);
			} else {
				pthread_mutex_unlock(&lock);
				wunit[i].nodes = perft(&wunit[i].pos, wunit[i].plydepth);
			}
		}

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
	pthread_id = malloc(sizeof(*pthread_id) * threads);

	for (i = 0; i < threads; i++) {
		thread_id[i] = i;
		/* Create the thread */
		pthread_create(&pthread_id[i], NULL, start_routine, (void*)&thread_id[i]);
		dbg("thread %d created\n", thread_id[i]);
	}
}

void randk_verify(void)
{
	int i;
	u64 x, n1, n2;
	n1 = 13596816608992115578ULL;
	n2 = 5033346742750153761ULL;

	printf("Verifying the PRNG...\n");

	/* Generate 10^9 B64MWC()s */
	for (i = 0; i < 1000000000; i++)
		x = B64MWC();
	if (x == n1)
		printf("Test 1: B64MWC(): OK: expected %"PRIu64", got %"PRIu64"\n", n1, x);
	else
		fatal("Test 1: B64MWC(): FAIL: expected %"PRIu64", got %"PRIu64"", n1, x);

	/* Generate 10^9 KISSes: */
	for (i = 0; i < 1000000000; i++)
		x = randk();
	if (x == n2)
		printf("Test 2: randk(): OK: expected %"PRIu64", got %"PRIu64"\n", n2, x);
	else
		fatal("Test 2: randk(): FAIL: expected %"PRIu64", got %"PRIu64"", n2, x);
}
