#include "move.h"

u64 MOVES_K[64];
u64 MOVES_Q[64];
u64 MOVES_R[64];
u64 MOVES_X[64];
u64 MOVES_N[64];
u64 ATTACKS_P[2][64];

u32 BITS_SQ1		= 0x3fUL << SHF_SQ1;
u32 BITS_SQ2		= 0x3fUL << SHF_SQ2;
u32 BITS_MOVE_TYPE	= 0x7UL << SHF_MOVE_TYPE;
u32 BITS_MPIECE		= 0xfUL << SHF_MPIECE;
u32 BITS_CPIECE		= 0xfUL << SHF_CPIECE;
u32 BITS_PPIECE		= 0xfUL << SHF_PPIECE;

u32 BITS_U_CASR		= 0xfUL << U_SHF_CASR;
u32 BITS_U_EP_SQ	= 0x7fUL << U_SHF_EP_SQ;
u32 BITS_U_FMR		= 0x7fUL << U_SHF_FMR;

void init_bitmasks_moves(void)
{
	int sq1, sq2, sq3, direction;
	int f1, f2, r1, r2;
	int i;
	int knight_moves[8] = {
		GO_NWW, GO_NEE, GO_NNW, GO_NNE, GO_SEE, GO_SWW, GO_SSE, GO_SSW
	};
	for (sq1 = A1; sq1 != SQ_NONE; sq1++) {
		MOVES_N[sq1] = 0x0ULL;
		for (i = 0; i < 8; i++) {
			sq2 = sq1 + knight_moves[i];
			if (valid_sq(sq2) && dist_sq(sq1, sq2) == 2) {
				f1 = file_from(sq1);
				f2 = file_from(sq2);
				r1 = rank_from(sq1);
				r2 = rank_from(sq2);
				if (f1 != f2 && r1 != r2) {
					MOVES_N[sq1] |= BIT[sq2];
				}
			}
		}
	}

	for (sq1 = A1; sq1 != SQ_NONE; sq1++) {
		MOVES_K[sq1] = 0x0ULL;
		MOVES_Q[sq1] = 0x0ULL;
		MOVES_R[sq1] = 0x0ULL;
		MOVES_X[sq1] = 0x0ULL;
		for (sq2 = A1; sq2 != SQ_NONE; sq2++) {
			direction = GO_RAY[sq1][sq2];
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

void init_pawn_attacks(void)
{
	int us, sq, r;
	for (us = W; us != COLOR_NONE; us++) {
		for (sq = A1; sq != SQ_NONE; sq++) {
			ATTACKS_P[us][sq] = 0x0ULL;
			r = rank_from(sq);
			if (rel_rank(us, r) < RANK_8)
				ATTACKS_P[us][sq] = MOVES_X[sq] & BIT_RANK[r + (us == W ? 1 : -1)];
		}
	}
}

void disp_move(u32 *move_info)
{
	int sq1, sq2, pm, pc, pp, mt;
	char sq_str[3];
	sq1 = move_sq1(move_info);
	sq2 = move_sq2(move_info);
	pm = mpiece(move_info);
	pc = cpiece(move_info);
	pp = ppiece(move_info);
	mt = move_type(move_info);
	printf("%s", sq_to_str(sq1, sq_str));
	printf("%s ", sq_to_str(sq2, sq_str));
	printf(" %s  ", piece_to_str(pm, sq_str));
	printf(" %s  ", piece_to_str(pc, sq_str));
	printf(" %s  ", piece_to_str(pp, sq_str));
	switch (mt) {
	case MOVE_NORMAL:
		printf("      ");
		break;
	case MOVE_OO:
		printf(" O-O  ");
		break;
	case MOVE_OOO:
		printf("O-O-O ");
		break;
	case MOVE_EP_CREATE:
		printf("e.p.O ");
		break;
	case MOVE_EP_CAPTURE:
		printf("e.p.X ");
		break;
	case MOVE_PROM:
		printf("prom%s ", piece_to_str(pp, sq_str));
		break;
	default:
		printf(" ???? ");
		break;
	}
}
