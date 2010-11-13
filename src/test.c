#include "test.h"

void test_perft(const char *str, int plydepth)
{
	struct position pos;
	import_sfen(str, &pos);
	disp_pos(&pos);
	test_perft_display(&pos, plydepth);
}

void test_perft_display(struct position *pos, int plydepth)
{
	struct move mlist[256];
	u32 undo_info;
	int color, moves_initial, fmn, sfen_maxlen;
	char sfen[100] = {'\0'};
	int i, j;
	u64 subnodes, nodes;
	u64 t1, t2, t3;
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
	printf("Found %d moves from initial position\n\n", moves_initial);
	printf("no. move pmv pcp ppr mvtyp chk sfen of child node (plydepth %d)\n", plydepth - 1);
        printf("--- ---- --- --- --- ----- --- -------------------------------------------------\n");

	if (plydepth == 1) {
		for (i = 0; i < moves_initial; i++, pos->checkers = 0x0ULL) {
			move_do(pos, &mlist[i].info, &undo_info);
			if (i + 1 < 100)
				printf("0");
			if (i + 1 < 10)
				printf("0");
			printf("%d ", i + 1);
			disp_move(&mlist[i].info);
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

			printf("\n");
			for (j = 0; j < 100; j++)
				sfen[j] = '\0';
			move_undo(pos, &mlist[i].info, &undo_info);
		}
	} else {
		t1 = get_time(); /* record time */
		/*
		 * If plydepth > 1, then try out each move, recursively, for
		 * until depth is 0 (or checkmate) --- and report back the total
		 * nodecount for each move
		 */
		for (i = 0; i < moves_initial; i++, pos->checkers = 0x0ULL) {
			move_do(pos, &mlist[i].info, &undo_info);
			if (i + 1 < 100)
				printf("0");
			if (i + 1 < 10)
				printf("0");
			printf("%d ", i + 1);
			disp_move(&mlist[i].info);
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

			/* Recurse into this move, up to plydepth times */
			subnodes = perft(pos, plydepth - 1);

			printf("%"PRIu64"\n", subnodes);
			move_undo(pos, &mlist[i].info, &undo_info);
			nodes += subnodes;
		}
		t2 = get_time();
		t3 = t2 - t1;
		printf("\nNodes: %"PRIu64"\n", nodes);
		if (t3) {
			printf("Time: ");
			time_pretty(t3);
			printf("Knps: %"PRIu64"\n", nodes/t3/10);
		}
		printf("\n");
	}
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
