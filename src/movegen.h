#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "hash.h"
#include "io.h"
#include "types.h"
#include "position.h"
#include "move.h"

/* Master bitboard initialization */
extern void initialize(void);

/* Sliding piece move generation */
extern u64 attacks_table_R[102400];
extern int attacks_idx_R[64];
extern u64 mask_R[64];
extern const int shift_R[64];
extern const u64 mult_R[64];

extern u64 attacks_table_X[5248];
extern int attacks_idx_X[64];
extern u64 mask_X[64];
extern const int shift_X[64];
extern const u64 mult_X[64];

extern u64 attacks_pseudo_R[64];
extern u64 attacks_pseudo_X[64];
extern u64 attacks_pseudo_Q[64];

static inline void set_virgin_rook_changed(int color, int sq_rook, u64 *pos_info)
{
	if (can_OO(color, pos_info) &&
	    sq_rook == sq_from(birthfile_H(pos_info), rel_rank(color, RANK_1)))
		revoke_OO(color, pos_info);
	else if (can_OOO(color, pos_info) &&
		 sq_rook == sq_from(birthfile_A(pos_info), rel_rank(color, RANK_1)))
		revoke_OOO(color, pos_info);
}

static inline u32 move_create_normal(int sq1, int sq2, int pm, int pc)
{
	return (u32)(
		     (u32)sq1 << SHF_SQ1		|
		     (u32)sq2 << SHF_SQ2		|
		     (u32)MOVE_NORMAL << SHF_MOVE_TYPE	|
		     (u32)pm << SHF_MPIECE		|
		     (u32)pc << SHF_CPIECE		|
		     (u32)PIECE_NONE << SHF_PPIECE
		     );
}

static inline u32 move_create(int sq1, int sq2, u8 movetype, int pm, int pc, int pp)
{
	return (u32)(
		     (u32)sq1 << SHF_SQ1		|
		     (u32)sq2 << SHF_SQ2		|
		     (u32)movetype << SHF_MOVE_TYPE	|
		     (u32)pm << SHF_MPIECE		|
		     (u32)pc << SHF_CPIECE		|
		     (u32)pp << SHF_PPIECE
		     );
}

static inline u64 attacks_R(int s, u64 blockers)
{
        u64 b = blockers & mask_R[s];
        return attacks_table_R[attacks_idx_R[s] + ((b * mult_R[s]) >> shift_R[s])];
}

static inline u64 attacks_X(int s, u64 blockers)
{
        u64 b = blockers & mask_X[s];
        return attacks_table_X[attacks_idx_X[s] + ((b * mult_X[s]) >> shift_X[s])];
}

static inline u64 attacks_Q(int s, u64 blockers)
{
        return attacks_R(s, blockers) | attacks_X(s, blockers);
}

static inline void bit_attacks_N(int attacker, struct position *pos, u64 *bits)
{
	u64 bb = pos->piece[attacker][N];
	while (bb) {
		*bits |= MOVES_N[pop_LSB(&bb)];
	}
}

static inline void bit_attacks_K(int attacker, struct position *pos, u64 *bits)
{
	*bits |= MOVES_K[sq_from_bit(&pos->piece[attacker][K])];
}

static inline void bit_attacks_P(int attacker, struct position *pos, u64 *bits)
{
        *bits |= ((pos->piece[attacker][P] << 7) >> (attacker << 4)) & ~BIT_FILE_H;
        *bits |= ((pos->piece[attacker][P] << 9) >> (attacker << 4)) & ~BIT_FILE_A;
}

static inline void bit_attacks_R(int attacker, struct position *pos, u64 *bits)
{
	u64 bb = pos->piece[attacker][R];
	while (bb) {
		*bits |= attacks_R(pop_LSB(&bb), pos->occupied);
	}
}

static inline void bit_attacks_X(int attacker, struct position *pos, u64 *bits)
{
	u64 bb = pos->piece[attacker][X];
	while (bb) {
		*bits |= attacks_X(pop_LSB(&bb), pos->occupied);
	}
}

static inline void bit_attacks_Q(int attacker, struct position *pos, u64 *bits)
{
	u64 bb = pos->piece[attacker][Q];
	while (bb) {
		*bits |= attacks_Q(pop_LSB(&bb), pos->occupied);
	}
}

/* covers only (regular) King and Knight moves so far */
static inline u64 bits_attacked_by(int attacker, struct position *pos)
{
	u64 bits = 0x0ULL;
	bit_attacks_N(attacker, pos, &bits);
	bit_attacks_K(attacker, pos, &bits);
	bit_attacks_P(attacker, pos, &bits);
	bit_attacks_R(attacker, pos, &bits);
	bit_attacks_X(attacker, pos, &bits);
	bit_attacks_Q(attacker, pos, &bits);
	return bits;
}

static inline void find_pinned(struct position *pos)
{
        int us, sqk, sq2;
        u64 pinners_possibly, ray;

        pos->pinned = 0x0ULL;
	us = pos->info & BIT_TURN;
	sqk = sq_from_bit(&pos->piece[us][K]);
	sq2 = SQ_NONE;
        pinners_possibly = attacks_Q(sqk, sliders(!us, pos)) & sliders(!us, pos);
        ray = 0x0ULL;

        while (pinners_possibly) {
                sq2 = pop_LSB(&pinners_possibly);
                ray = BITS_Q[sqk][sq2] & pos->occupied;
		/* Is there only 1 of our pieces in the ray (b/n enemy slider
		 * and our king?) */
                if (ray && (!(ray & (ray - 1)))) {
			/* Correctly match the ray type with the right slider */
			switch (pos->piece_on[sq2]) {
			case R:
				if (BITS_R[sqk][sq2])
					pos->pinned |= ray;
				break;
			case X:
				if (BITS_X[sqk][sq2])
					pos->pinned |= ray;
				break;
			case Q:
				pos->pinned |= ray;
				break;
			default:
				assert(0);
				break;
			}
                }
        }
}

static inline void find_checkers(struct position *pos)
{
	u64 blockers;
	int us, sqk, sq2;

	pos->checkers = 0x0ULL;
	blockers = 0x0ULL;
	us = pos->info & BIT_TURN;
	sqk = sq_from_bit(&pos->piece[us][K]);
	sq2 = SQ_NONE;

	/* find all enemy checkers against our king */
	if (bits_attacked_by(!us, pos) & BIT[sqk]) {
		/* add all knight, pawn checkers */
		pos->checkers |= MOVES_N[sqk] & pos->piece[!us][N];
		pos->checkers |= ATTACKS_P[us][sqk] & pos->piece[!us][P];
		/* add slideing checkers, only if not blocked */
		u64 enemy_sliders = attacks_Q(sqk, pos->occupied) & sliders(!us, pos);
		while (enemy_sliders) {
			sq2 = pop_LSB(&enemy_sliders);
			blockers = BITS_Q[sqk][sq2] & pos->occupied;
			if (!blockers) {
				switch (pos->piece_on[sq2]) {
				case R:
					if (BITS_R[sqk][sq2])
						pos->checkers |= BIT[sq2];
					/* adjacent rook */
					if (MOVES_K[sqk] & MOVES_R[sqk] & BIT[sq2])
						pos->checkers |= BIT[sq2];
					break;
				case X:
					if (BITS_X[sqk][sq2])
						pos->checkers |= BIT[sq2];
					/* adjacent bishop */
					if (MOVES_K[sqk] & MOVES_X[sqk] & BIT[sq2])
						pos->checkers |= BIT[sq2];
					break;
				case Q:
					pos->checkers |= BIT[sq2];
					/* adjacent queen */
					if (MOVES_K[sqk] & BIT[sq2])
						pos->checkers |= BIT[sq2];
					break;
				default:
					assert(0);
				}
			}
		}
	}
}

extern void init_attacks(void);
extern void init_attacks_sliding(u64 attacks[], int attack_idx[], u64 mask[], const int shift[2], const u64 mult[], int delta[][2]);
extern u64 sliding_attacks(int sq, u64 block, int dirs, int delta[][2], int fmin, int fmax, int rmin, int rmax);
extern u64 index_to_bitboard(int idx, u64 mask);

/* Generic move generation */
extern u8 movegen(struct position *pos, struct move *mlist, int us);
extern u8 movegen_N(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq1, int sq2, u64 *source, int pc);
extern u8 movegen_K(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq2, int pc);
extern u8 movegen_P(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq1, int sq2, u64 *source, int pc);
extern u8 movegen_slider(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq1, int sq2, u64 *source, int pc, int piece_slider);
extern void move_do(struct position *pos, u32 *move_info, u32 *undo_info);
extern void move_undo(struct position *pos, u32 *move_info, u32 *undo_info);

#endif
