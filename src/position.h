#ifndef POSITION_H
#define POSITION_H

#include "io.h"
#include "types.h"
#include "move.h"

/* Position */
struct position {
	u64 piece[12];	/* all color/piece combinations */
	u64 pieces[2];	/* all White and Black pieces */
	u64 occupied;	/* all occupied squares */
	u64 pinned;	/* pinned pieces (for the side to move) */
	u64 checkers;	/* all enemy attackers against our king */
	u64 info;	/* metadata about position */
	u64 zkey;	/* zobrist key */
	u8 piece_on[64];	/* which squares have what pieces */
};

/*
 * Bit map of position->info variable.
 *
 * MSB		Bits Used	Information
 * ---		---------	-----------
 * .		37		*Unused*
 * .		7		Fifty Move Rule counter (plies)
 * .		7		Square of en passant pawn (SQ_NONE if none)
 * .		3		A-side rook's starting file (can be FILE_A-FILE_F)
 * .		3		H-side rook's starting file (can be FILE_C-FILE_H)
 * .		1		Black's A-side castling right (O-O-O)
 * .		1		Black's H-side castling right (O-O)
 * .		1		White's A-side castling right (O-O-O)
 * .		1		White's H-side castling right (O-O)
 * .		1		Is the side to move in check? (1 = in check)
 * .		1		Turn, aka side to move (0 = White to move; 1 = Black)
 * LSB
 */

/* The number of bits we need to shift right in order to read the value of the
 * field in question.
 */
#define SHF_TURN		0
#define SHF_IN_CHECK		1
#define SHF_OO_W		2
#define SHF_OOO_W		3
#define SHF_OO_B		4
#define SHF_OOO_B		5
#define SHF_HROOK_FILE		6
#define SHF_AROOK_FILE		9
#define SHF_EP_SQ		12
#define SHF_FMR			19

extern u64 BIT_TURN;	/* 0 is White to move; 1 for Black */
extern u64 BIT_IN_CHECK;
extern u64 BIT_OO_W;
extern u64 BIT_OOO_W;
extern u64 BIT_OO_B;
extern u64 BIT_OOO_B;
extern u64 BITS_HROOK_FILE;
extern u64 BITS_AROOK_FILE;
extern u64 BITS_EP_SQ;
extern u64 BITS_FMR;

extern void import_sfen(const char *str, struct position *pos);

extern void pos_clear(struct position *pos);
extern void disp_pos(struct position *pos);
extern void disp_bitboard(u64 bb);
void set_pieces_rank(int rank, struct position *pos, char *pieces);

#endif
