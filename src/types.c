#include "types.h"

int GO_RAY[64][64];
u64 BIT[64];
u64 BIT_FILE_RANK[8][8];
u64 BITS_R[64][64];
u64 BITS_X[64][64];
u64 BITS_Q[64][64];
u64 BITS_Q_INCL[64][64];

void init_bitmasks()
{
	int rank, file, sq, sq1, sq2, sq3, sq4, direction, i;
	int ray[4] = {
		GO_NE, GO_SE, GO_SW, GO_NW
	};

	/* BIT[64] */
	for (sq = A1; sq != SQ_NONE; sq++)
		BIT[sq] = 0x1ULL << sq;

	/* BIT_FILE_RANK[8][8] */
	for (file = FILE_A; file != FILE_NONE; file++) {
		for (rank = RANK_1; rank != RANK_NONE; rank++) {
			BIT_FILE_RANK[file][rank] = BIT_FILE[file] & BIT_RANK[rank];
		}
	}

	/* GO_RAY[64][64] */
	for (sq1 = A1; sq1 != SQ_NONE; sq1++) {
		for (sq2 = A1; sq2 != SQ_NONE; sq2++) {
			GO_RAY[sq1][sq2] = GO_NONE;
			if (sq1 == sq2)
				continue;
			/* same file */
			if (file_from(sq1) == file_from(sq2))
				GO_RAY[sq1][sq2] = (sq2 > sq1) ? GO_N : GO_S;
			/* same rank */
			else if (rank_from(sq1) == rank_from(sq2))
				GO_RAY[sq1][sq2] = (sq2 > sq1) ? GO_E : GO_W;

			for (i = 0; i < 4; i++) {
				direction = ray[i];
				for (sq3 = sq1, sq4 = sq1 + direction; sq4 != sq2 && valid_sq(sq4) && dist_sq(sq3, sq4) == 1; sq3 += direction, sq4 += direction);
				if (sq4 == sq2 && dist_sq(sq3, sq4) == 1) {
					GO_RAY[sq1][sq2] = direction;
					break;
				}
			}
		}
	}

	/* BITS_Q[64][64], BITS_Q_INCL[64][64], BITS_R[64], BITS_X[64] */
	for (sq1 = A1; sq1 != SQ_NONE; sq1++) {
		for (sq2 = A1; sq2 != SQ_NONE; sq2++) {
			BITS_Q[sq1][sq2] = 0x0ULL;
			BITS_Q_INCL[sq1][sq2] = 0x0ULL;
			BITS_R[sq1][sq2] = 0x0ULL;
			BITS_X[sq1][sq2] = 0x0ULL;

			direction = GO_RAY[sq1][sq2];
			if (direction != GO_NONE) {
				/* Bits between sq1 and sq2, if any */
				for (sq3 = sq1 + direction; sq3 != sq2; sq3 += direction) {
					BITS_Q[sq1][sq2] |= BIT[sq3];
					BITS_Q_INCL[sq1][sq2] |= BIT[sq3];
					if (direction == GO_N ||
					    direction == GO_E ||
					    direction == GO_S ||
					    direction == GO_W) {
						BITS_R[sq1][sq2] |= BIT[sq3];
					} else
						BITS_X[sq1][sq2] |= BIT[sq3];
				}
				/* Bits for sq1 and sq2 themselves */
				BITS_Q_INCL[sq1][sq2] |= BIT[sq1] | BIT[sq2];
			}
		}
	}
}

char *sq_to_str(int sq, char *str)
{
	int f, r;
	str[2] = '\0';
	f = file_from(sq);
	r = rank_from(sq);
	switch (f) {
	case FILE_A: str[0] = 'a'; break;
	case FILE_B: str[0] = 'b'; break;
	case FILE_C: str[0] = 'c'; break;
	case FILE_D: str[0] = 'd'; break;
	case FILE_E: str[0] = 'e'; break;
	case FILE_F: str[0] = 'f'; break;
	case FILE_G: str[0] = 'g'; break;
	case FILE_H: str[0] = 'h'; break;
	}
	switch (r) {
	case RANK_1: str[1] = '1'; break;
	case RANK_2: str[1] = '2'; break;
	case RANK_3: str[1] = '3'; break;
	case RANK_4: str[1] = '4'; break;
	case RANK_5: str[1] = '5'; break;
	case RANK_6: str[1] = '6'; break;
	case RANK_7: str[1] = '7'; break;
	case RANK_8: str[1] = '8'; break;
	}

	return str;
}

char *piece_to_str(int piece, char *str)
{
	str[1] = '\0';
	switch (piece) {
	case K: str[0] = 'K'; break;
	case Q: str[0] = 'Q'; break;
	case R: str[0] = 'R'; break;
	case X: str[0] = 'B'; break;
	case N: str[0] = 'N'; break;
	case P: str[0] = '*'; break;
	case PIECE_NONE: str[0] = ' '; break;
	default: break;
	}

	return str;
}
