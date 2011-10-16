#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>	/* uint64_t, etc. */
#include <stdbool.h>	/* bool type */
#include "io.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)

#define FLIP_SQ_VERT 56

/* Pieces */
enum {
	K, Q, R, X, N, P,
	PIECE_NONE
};

/* Colors */
enum {
	W, B, COLOR_NONE
};


/* Files and Ranks */
enum {
	FILE_A,
	FILE_B,
	FILE_C,
	FILE_D,
	FILE_E,
	FILE_F,
	FILE_G,
	FILE_H,
	FILE_NONE
};

enum {
	RANK_1,
	RANK_2,
	RANK_3,
	RANK_4,
	RANK_5,
	RANK_6,
	RANK_7,
	RANK_8,
	RANK_NONE
};

/* Squares */
enum {
        A1, B1, C1, D1, E1, F1, G1, H1, // A1 is 0
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8, // H8 is 63
        SQ_NONE
};

/* Number of shifts on a bit that would result in a King's move (the 8
 * rays) or a Knight's move (again, 8 directions) */
enum {
	/* King */
	GO_N = 8, GO_E = 1,
	GO_S = -8, GO_W = -1,
	GO_NE = 9, GO_SE = -7,
	GO_SW = -9, GO_NW = 7,
	/* Knight */
	GO_NWW = 6, GO_NEE = 10,
	GO_NNW = 15, GO_NNE = 17,
	GO_SEE = -6, GO_SWW = -10,
	GO_SSE = -15, GO_SSW = -17,
	GO_NONE = 0
};

#define BITS_ALL	0xffffffffffffffffULL

#define BIT_FILE_A	0x0101010101010101ULL
#define BIT_FILE_B	0x0202020202020202ULL
#define BIT_FILE_C	0x0404040404040404ULL
#define BIT_FILE_D	0x0808080808080808ULL
#define BIT_FILE_E	0x1010101010101010ULL
#define BIT_FILE_F	0x2020202020202020ULL
#define BIT_FILE_G	0x4040404040404040ULL
#define BIT_FILE_H	0x8080808080808080ULL
static const u64 BIT_FILE[8] = {
	BIT_FILE_A,
	BIT_FILE_B,
	BIT_FILE_C,
	BIT_FILE_D,
	BIT_FILE_E,
	BIT_FILE_F,
	BIT_FILE_G,
	BIT_FILE_H
};

#define BIT_RANK_1	0x00000000000000ffULL
#define BIT_RANK_2	0x000000000000ff00ULL
#define BIT_RANK_3	0x0000000000ff0000ULL
#define BIT_RANK_4	0x00000000ff000000ULL
#define BIT_RANK_5	0x000000ff00000000ULL
#define BIT_RANK_6	0x0000ff0000000000ULL
#define BIT_RANK_7	0x00ff000000000000ULL
#define BIT_RANK_8	0xff00000000000000ULL
static const u64 BIT_RANK[8] = {
	BIT_RANK_1,
	BIT_RANK_2,
	BIT_RANK_3,
	BIT_RANK_4,
	BIT_RANK_5,
	BIT_RANK_6,
	BIT_RANK_7,
	BIT_RANK_8
};

/* Files that are adjacent to given FILE, excluding the given FILE itself */
static const u64 BIT_FILES_ADJACENT[8] = {
	BIT_FILE_B,
	BIT_FILE_A | BIT_FILE_C,
	BIT_FILE_B | BIT_FILE_D,
	BIT_FILE_C | BIT_FILE_E,
	BIT_FILE_D | BIT_FILE_F,
	BIT_FILE_E | BIT_FILE_G,
	BIT_FILE_F | BIT_FILE_H,
	BIT_FILE_G,
};

extern int GO_RAY[64][64];	/* Direction of sq1 to sq2, if on a ray */
extern u64 BIT[64];		/* 1 bit turned on for each square */
extern u64 BIT_FILE_RANK[8][8]; /* same as BIT[64], but 2-dimensional */

extern u64 BITS_R[64][64];	/* if squares are on cardinal (rook) ray, then
				   it holds bits between those two squares */
extern u64 BITS_X[64][64];	/* same as BITS_R[64][64], but for diagonals */
extern u64 BITS_Q[64][64];	/* BITS_R and BITS_X combined (any ray) */
extern u64 BITS_Q_INCL[64][64];	/* same as BITS_Q[64][64], but also includes the
				   two squares */

#define DE_BRUIJN_SEQUENCE 0x0218a392dd5ecd3fULL

static const int LSB_TABLE[64] = {
    0,  1,  2,  7,  3, 13,  8, 19,
    4, 25, 14, 28,  9, 50, 20, 56,
    5, 17, 26, 54, 15, 38, 29, 40,
   10, 47, 51, 31, 21, 34, 42, 57,
   63,  6, 12, 18, 24, 27, 49, 55,
   16, 53, 37, 39, 46, 30, 33, 41,
   62, 11, 23, 48, 52, 36, 45, 32,
   61, 22, 35, 44, 60, 43, 59, 58
};

/* Convert a bitboard's LSB into a square */
static inline int sq_from_bits(u64 *b)
{
	assert(*b);
	return LSB_TABLE[(DE_BRUIJN_SEQUENCE * (*b & -(*b))) >> 58];
}

/* Convert a single bit back into a square */
static inline int sq_from_bit(u64 *b)
{
	assert(*b);
	return LSB_TABLE[(DE_BRUIJN_SEQUENCE * *b) >> 58];
}

static inline int pop_LSB(u64 *b)
{
        int sq_LSB = LSB_TABLE[(DE_BRUIJN_SEQUENCE * (*b & -(*b))) >> 58];
        *b &= (*b - 1);	/* clear LSB */
        return sq_LSB;
}

static inline int sq_MSB(u64 b)
{
        int sq = A1;
        if (b > 0xffffffffULL) {
                b >>= 32;
                sq = 32;
        }
        if (b > 0xffffULL) {
                b >>= 16;
                sq += 16;
        }
        if (b > 0xffULL) {
                b >>= 8;
                sq += 8;
        }
        if (b > 0xfULL) {
                b >>= 4;
                sq += 4;
        }
        if (b > 0x3ULL) {
                b >>= 2;
                sq += 2;
        }
        if (b > 0x1ULL) {
                b >>= 1;
                sq += 1;
        }
        return sq;
}

static inline int bit_count(u64 b)
{
        b -= ((b >> 1) & 0x5555555555555555ULL);
        b = ((b >> 2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
        b = ((b >> 4) + b) & 0x0f0f0f0f0f0f0f0fULL;
        b *= 0x0101010101010101ULL;
        return b >> 56;
}

static inline int bit_count_max15(u64 b)
{
        b -= (b >> 1) & 0x5555555555555555ULL;
        b = ((b >> 2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
        b *= 0x1111111111111111ULL;
        return b >> 60;
}

static inline int sq_from(int f, int r)
{
	return f | (r << 3);
}

/* Modulo by 8; i.e., look only at the smallest 3 bits (7 is "111" in binary) */
static inline int file_from(int sq)
{
	return (sq & 7);
}

/* Divide the square by 8 (i.e., divide by 2, three times) */
static inline int rank_from(int sq)
{
	return (sq >> 3);
}

static inline int valid_rank(int rank)
{
	return (rank >= RANK_1 && rank <= RANK_8);
}

static inline int valid_file(int file)
{
	return (file >= FILE_A && file <= FILE_H);
}

static inline int valid_sq(int sq)
{
	return (sq >= A1 && sq <= H8);
	//return (valid_file(file_from(sq)) && valid_rank(rank_from(sq)));
}

static inline int dist_file(int sq1, int sq2)
{
	return (abs(file_from(sq1) - file_from(sq2)));
}

static inline int dist_rank(int sq1, int sq2)
{
	return (abs(rank_from(sq1) - rank_from(sq2)));
}

static inline int dist_sq(int sq1, int sq2)
{
	return (MAX(dist_file(sq1, sq2), dist_rank(sq1, sq2)));
}

static inline int rel_rank(int color, int rank)
{
	return abs(rank - (color * 7));
}

static inline int rel_sq(int color, int sq)
{
	return sq ^ (color * FLIP_SQ_VERT);
}

static inline char file_to_char(int f)
{
	return 'a' + f;
}

static inline char rank_to_char(int r)
{
	return '1' + r;
}

extern void init_bitmasks(void);
extern char *sq_to_str(int sq, char *str);
extern char *piece_to_str(int piece, char *str);

#endif
