#include "move.h"

u64 MOVES_K[64];
u64 MOVES_Q[64];
u64 MOVES_R[64];
u64 MOVES_X[64];

void init_bitmasks_moves()
{
	int sq1, sq2, sq3, direction;

	for (sq1 = A1; sq1 != SQ_NONE; sq1++) {
		MOVES_K[sq1] = 0x0ULL;
		MOVES_Q[sq1] = 0x0ULL;
		MOVES_R[sq1] = 0x0ULL;
		MOVES_X[sq1] = 0x0ULL;
		for (sq2 = A1; sq2 != SQ_NONE; sq2++) {
			direction = GO[sq1][sq2];
			if (direction != GO_NONE) {
				if (dist_sq(sq1, sq2) == 1)
					MOVES_K[sq1] |= BIT[sq2];
				MOVES_Q[sq1] |= BIT[sq2];
				/* Bits between sq1 and sq2, if any */
				for (sq3 = sq1 + direction; sq3 != sq2; sq3 += direction) {
					if (direction == GO_N ||
					    direction == GO_E ||
					    direction == GO_S ||
					    direction == GO_W) {
						MOVES_R[sq1] |= BIT[sq3];
					} else
						MOVES_X[sq1] |= BIT[sq3];
				}
				/* sq2 itself */
				if (direction == GO_N ||
				    direction == GO_E ||
				    direction == GO_S ||
				    direction == GO_W) {
					MOVES_R[sq1] |= BIT[sq2];
				} else
					MOVES_X[sq1] |= BIT[sq2];
			}
		}
	}
}
