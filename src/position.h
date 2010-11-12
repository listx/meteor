#ifndef POSITION_H
#define POSITION_H

#include "io.h"
#include "types.h"

/* Position */
struct position {
	u64 piece[2][6];	/* all color/piece combinations */
	u64 pieces[2];		/* all White and Black pieces */
	u64 occupied;		/* all occupied squares */
	u64 pinned;		/* pinned pieces (for the side to move) */
	u64 checkers;		/* all enemy attackers against our king */
	u64 info;		/* metadata about position */
	u64 zkey;		/* zobrist key */
	u8 piece_on[64];	/* which squares have what pieces */
};

/* keeps track of full move number of current root position */
int FULL_MOVE_NUMBER;

/*
 * Bitmap of position->info variable.
 *
 * MSB		Bits Used	Information
 * ---		---------	-----------
 * .		37		*Unused*
 * .		7		Fifty Move Rule counter (plies)
 * .		7		Square of en passant pawn (SQ_NONE if none, so
 *				  need 7 bits)
 * .		3		A-side rook's starting file (can be FILE_A-FILE_F)
 * .		3		H-side rook's starting file (can be FILE_C-FILE_H)
 * .		1		Black's A-side castling right (O-O-O)
 * .		1		Black's H-side castling right (O-O)
 * .		1		White's A-side castling right (O-O-O)
 * .		1		White's H-side castling right (O-O)
 * .		1		Turn, aka side to move (0 = White to move; 1 = Black)
 * LSB
 */

/* The number of bits we need to shift right in order to read the value of the
 * field in question.
 */
#define SHF_TURN		0
#define SHF_OO_W		1
#define SHF_OOO_W		2
#define SHF_OO_B		3
#define SHF_OOO_B		4
#define SHF_HROOK_FILE		5
#define SHF_AROOK_FILE		8
#define SHF_EP_SQ		11
#define SHF_FMR			18

#define SHF_CASR		1

extern u64 BIT_TURN;	/* 0 is White to move; 1 for Black */
extern u64 BIT_OO_W;
extern u64 BIT_OOO_W;
extern u64 BIT_OO_B;
extern u64 BIT_OOO_B;
extern u64 BITS_HROOK_FILE;
extern u64 BITS_AROOK_FILE;
extern u64 BITS_EP_SQ;
extern u64 BITS_FMR;

extern u64 BITS_CASR;

/*
 * Read data
 */

static inline int our_color(u64 *pos_info)
{
	return (*pos_info & BIT_TURN) >> SHF_TURN;
}

static inline int can_OO_W(u64 *pos_info)
{
	return (*pos_info & BIT_OO_W) >> SHF_OO_W;
}

static inline int can_OOO_W(u64 *pos_info)
{
	return (*pos_info & BIT_OOO_W) >> SHF_OOO_W;
}

static inline int can_OO_B(u64 *pos_info)
{
	return (*pos_info & BIT_OO_B) >> SHF_OO_B;
}

static inline int can_OOO_B(u64 *pos_info)
{
	return (*pos_info & BIT_OOO_B) >> SHF_OOO_B;
}

static inline int can_OO(int color, u64 *pos_info)
{
	return (color == W) ? can_OO_W(pos_info) : can_OO_B(pos_info);
}

static inline int can_OOO(int color, u64 *pos_info)
{
	return (color == W) ? can_OOO_W(pos_info) : can_OOO_B(pos_info);
}

static inline int can_OO_or_OOO(int color, u64 *pos_info)
{
	return (color == W) ? (*pos_info & (0x3ULL << SHF_OO_W)) : (*pos_info & (0x3ULL << SHF_OO_B));
}

static inline u8 cas_rights(u64 *pos_info)
{
	return (*pos_info & BITS_CASR) >> SHF_CASR;
}

static inline int birthfile_H(u64 *pos_info)
{
	return (*pos_info & BITS_HROOK_FILE) >> SHF_HROOK_FILE;
}

static inline int birthfile_A(u64 *pos_info)
{
	return (*pos_info & BITS_AROOK_FILE) >> SHF_AROOK_FILE;
}

static inline int ep_sq(u64 *pos_info)
{
	return (*pos_info & BITS_EP_SQ) >> SHF_EP_SQ;
}

static inline int plies_FMR(u64 *pos_info)
{
	return (*pos_info & BITS_FMR) >> SHF_FMR;
}

static inline u64 sliders_R(int color, struct position *pos)
{
	return (
		pos->piece[color][R] |
		pos->piece[color][Q]
	       );
}

static inline u64 sliders(int color, struct position *pos)
{
	return (
		pos->piece[color][R] |
		pos->piece[color][X] |
		pos->piece[color][Q]
	       );
}

/*
 * Write data
 */

static inline void swap_turn(u64 *pos_info)
{
	*pos_info ^= BIT_TURN;
}

static inline void set_ep_sq(int sq, u64 *pos_info)
{
	*pos_info &= ~BITS_EP_SQ;
	*pos_info |= (u64)(sq) << SHF_EP_SQ;
}

static inline void set_birthfile_H(int f, u64 *pos_info)
{
	*pos_info |= (u64)f << SHF_HROOK_FILE;
}

static inline void set_birthfile_A(int f, u64 *pos_info)
{
	*pos_info &= ~BITS_AROOK_FILE;
	*pos_info |= (u64)f << SHF_AROOK_FILE;
}

static inline void revoke_OO_W(u64 *pos_info)
{
	*pos_info &= ~BIT_OO_W;
}

static inline void revoke_OOO_W(u64 *pos_info)
{
	*pos_info &= ~BIT_OOO_W;
}

static inline void revoke_OO_B(u64 *pos_info)
{
	*pos_info &= ~BIT_OO_B;
}

static inline void revoke_OOO_B(u64 *pos_info)
{
	*pos_info &= ~BIT_OOO_B;
}

static inline void revoke_OO_OOO_W(u64 *pos_info)
{
	*pos_info &= ~(0x3ULL << SHF_OO_W);
}

static inline void revoke_OO_OOO_B(u64 *pos_info)
{
	*pos_info &= ~(0x3ULL << SHF_OO_B);
}

static inline void revoke_OO(int color, u64 *pos_info)
{
	(color == W) ? revoke_OO_W(pos_info) : revoke_OO_B(pos_info);
}

static inline void revoke_OOO(int color, u64 *pos_info)
{
	(color == W) ? revoke_OOO_W(pos_info) : revoke_OOO_B(pos_info);
}

static inline void revoke_OO_OOO(int color, u64 *pos_info)
{
	(color == W) ? revoke_OO_OOO_W(pos_info) : revoke_OO_OOO_B(pos_info);
}

static inline void reset_FMR(u64 *pos_info)
{
	*pos_info &= ~BITS_FMR;
}

static inline void set_FMR(int plies, u64 *pos_info)
{
	*pos_info &= ~BITS_FMR;
	*pos_info |= ((u64)plies) << SHF_FMR;
}

static inline void inc_FMR(u64 *pos_info)
{
	if (plies_FMR(pos_info) < 100)
		set_FMR((plies_FMR(pos_info) + 1), pos_info);
}

extern void import_sfen(const char *str, struct position *pos);
extern char *export_sfen(struct position *pos, char *str, int fmn);
extern void pos_clear(struct position *pos);
extern void disp_pos(struct position *pos);
extern void disp_bitboard(u64 bb);
void set_pieces_rank(int rank, struct position *pos, char *pieces);

#endif
