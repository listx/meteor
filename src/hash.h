#ifndef HASH_H
#define HASH_H

#include "io.h"
#include "random.h"

struct zobrist {
	u64 piece[2][6][64];
	u64 casr[16];
	u64 ep_sq[65]; /* en passant sq; SQ_NONE if no square */
	u64 turn;
};

extern struct zobrist zob;

struct tt_perft_bucket {
	u64 zkey;
	u64 info; /* first 4 bits is depth, and next 60 bits is nodecount */
};

/*
 * Transposition table; more accurately, this is simply a hashtable that stores
 * the position (zobrist key) and a value (depth + nodes)
 */
struct tt_perft {
        struct tt_perft_bucket *bucket;
        u32 entries; /* number of entries */
        u64 writes; /* number of writes to the tt (global counter) */
};

extern struct tt_perft tt;

/* Read tt bucket */
static inline int get_depth(u64 *tt_info)
{
	return (int)(*tt_info & 0xf);
}
static inline u64 get_nodes(u64 *tt_info)
{
	return (u64)(*tt_info >> 4);
}

/*
 * Each zobrist key is hashed to a single "entry", which is just a
 * human-friendly term for 4 consecutive elements (aka "buckets") of the
 * hashtable array.  Four seems to be the optimal number of buckets in order to
 * balance the need to reduce hash collisions against the need to resolve hash
 * collisions when we run out of buckets.
 *
 * This function returns the memory address of the first hash bucket (of
 * possible 4 buckets) of a given zkey. Since a zkey is 64 bits wide, and
 * because our hash table does not use 2^(64-1) buckets (that would be 18
 * exabytes!), this means our hash function needs to reduce the large domain of
 * zkeys to what our hash table size actually is. We do this by effectively
 * reducing the 64-bit wide zkey to exactly the size of the hash table; i.e.,
 * instead of reading each zkey's entire 64 bits, we only read the lowest 15, or
 * 16 bits (the larger the hash table size, the more bits we read). That's what
 * the (*zkey & (tt.buckets - 1)) does. Then, we multiply by 4 (by doing two
 * operations of ^2 (two left-shifts)) because there are 4 "buckets" for each
 * zkey entry --- so the very first bucket is at memory address (tt.bucket +
 * 0), then the next one is at (tt.bucket + 4), then at (tt.bucket + 8), and
 * so on.
 */
static inline struct tt_perft_bucket *first_bucket(u64 *zkey)
{
        return tt.bucket + ((*zkey & (tt.entries - 1)) << 2);
}

static inline void set_bucket_lockless(struct tt_perft_bucket *bucket, u64 *zkey, u64 *nodes, u64 *depth)
{
	u64 info = ((*nodes << 4) | *depth);
	bucket->zkey = *zkey ^ *depth;
	bucket->info = info;
}

static inline u32 tt_hash_size(void)
{
	return tt.entries * sizeof(*tt.bucket) * 4 ;
}

extern void init_zob(void);
extern void show_zob(void);
extern void clear_tt(void);
extern void free_tt(void);
extern void init_tt_perft(int desiredMB);
extern struct tt_perft_bucket *get_bucket_lockless(u64 *zkey, u64 *info);
extern void update_entry(u64 *zkey, u64 *nodes, int *depth);

#endif
