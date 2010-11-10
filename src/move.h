#ifndef MOVE_H
#define MOVE_H

#include "io.h"
#include "types.h"

extern u64 MOVES_K[64];
extern u64 MOVES_Q[64];
extern u64 MOVES_R[64];
extern u64 MOVES_X[64];
extern u64 MOVES_N[64];

/* Move Information */
struct move {
	u32 info;	/* metadata about move */
	int score;	/* strength of move */
};

/*
 * Bitmap of move->info variable.
 *
 * MSB		Bits Used	Information
 * ---		---------	-----------
 * .		5		*Unused*
 * .		4		Piece type of promoted piece (if any)
 * .		4		Piece type of captured piece (not necessarily sq2)
 * .		4		Piece type of moving piece (sq1)
 * .		3		Move type
 * .		6		Destination square (sq2)
 * .		6		Starting square (sq1)
 * LSB
 */

/* The number of bits we need to shift right in order to read the value of the
 * field in question.
 */
#define SHF_SQ1		0
#define SHF_SQ2		6
#define SHF_MOVE_TYPE	12
#define SHF_MPIECE	15
#define SHF_CPIECE	19
#define SHF_PPIECE	23

extern u32 BITS_SQ1;
extern u32 BITS_SQ2;
extern u32 BITS_MOVE_TYPE;
extern u32 BITS_MPIECE;
extern u32 BITS_CPIECE;
extern u32 BITS_PPIECE;

enum {
	MOVE_NORMAL,		/* captures, noncaptures */
	MOVE_OO,		/* H-side castling */
	MOVE_OOO,		/* A-side castling */
	MOVE_EP_CREATE,		/* creates en passant opportunity for other side */
	MOVE_EP_CAPTURE,	/* en passant capture */
	MOVE_PROM,		/* promotion; could itself be a capturing move */
	MOVE_NONE,
	MOVE_NULL
};

/*
 * Undo Move Information
 *
 * Undo info is stored in a plain u16 variable, not a struct like the other more
 * complex data structures like *position* and *move*.
 */

/*
 * Bitmap of undo (u32) variable.
 *
 * MSB		Bits Used	Information
 * ---		---------	-----------
 * .		14		*Unused*
 * .		7		FMR clock (plies)
 * .		7		En passant file
 * .		4		Castling rights snapshot of all colors
 * LSB
 */

static inline u32 move_sq1(u32 *move_info)
{
	return (*move_info & BITS_SQ1) >> SHF_SQ1;
}

static inline u32 move_sq2(u32 *move_info)
{
	return (*move_info & BITS_SQ2) >> SHF_SQ2;
}

static inline int move_type(u32 *move_info)
{
	return (*move_info & BITS_MOVE_TYPE) >> SHF_MOVE_TYPE;
}

static inline u32 mpiece(u32 *move_info)
{
	return (*move_info & BITS_MPIECE) >> SHF_MPIECE;
}

static inline u32 cpiece(u32 *move_info)
{
	return (*move_info & BITS_CPIECE) >> SHF_CPIECE;
}

static inline u32 ppiece(u32 *move_info)
{
	return (*move_info & BITS_PPIECE) >> SHF_PPIECE;
}

extern void init_bitmasks_moves();
extern void disp_move(u32 *move_info);

#endif
