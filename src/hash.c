#include "hash.h"

struct zobrist zob;
struct tt_perft tt;

void init_zob()
{
	int us, p, sq, i;
	u64 seed;

	seed = 0x2da51b891ae979f8; /* Fourmilab HotBits radioactive RNG (www.fourmilab.ch) */

	randk_seed_manual(seed);

	for (us = W; us != COLOR_NONE; us++)
		for (p = K; p != PIECE_NONE; p++)
			for (sq = A1; sq != SQ_NONE; sq++)
				zob.piece[us][p][sq] = randk();

	for (i = 0; i < 16; i++)
		zob.casr[i] = randk();

	for (i = 0; i < 65; i++)
		zob.ep_sq[i] = randk();

	zob.turn = randk();
}

void show_zob()
{
	int us, p, sq, i;

	for (us = W; us != COLOR_NONE; us++)
		for (p = K; p != PIECE_NONE; p++)
			for (sq = A1; sq != SQ_NONE; sq++)
				printf("zob.piece[%d][%d][%d] is: %"PRIu64"\n", us, p, sq, zob.piece[us][p][sq]);

	for (i = 0; i < 16; i++)
		printf("zob.casr[%d] is: %"PRIu64"\n", i, zob.casr[i]);

	for (i = 0; i < 64; i++)
		printf("zob.ep_sq[%d] is: %"PRIu64"\n", i, zob.ep_sq[i]);

	printf("zob.turn is: %"PRIu64"\n", zob.turn);
}

void clear_tt()
{
        memset(tt.bucket, 0, tt_hash_size());
        tt.writes = 0;
}

void free_tt()
{
	free(tt.bucket);
	tt.entries = 0;
}

void init_tt_perft(int desiredMB)
{
	u32 desired_bytes;

	desired_bytes = desiredMB << 20; /* covert MiB to B */

	/* Find how many MB we can *actually* use, based on the memory
	 * requirement of a tt.bucket value. However, instead of a raw maximum,
	 * we only consider neat intervals of even numbers, so that we end up
	 * with a nice, "computerish" number.
	 */
	tt.entries = 0;
        while (((tt.entries + 1024) * 4 * (sizeof(*tt.bucket))) <= (desired_bytes))
                tt.entries += 1024;

	/* Dynamically allocate the needed number of bytes; we multiply by 4
	 * because we use 4 "buckets" per entry */
	tt.bucket = malloc(tt_hash_size());

	if (tt.bucket == NULL)
		fatal("hashtable memory allocation failure (not enough RAM)\n");

	clear_tt();
}

/* Locate the correct hash entry's first available (overwriteable) bucket; we
 * verify the correctness of the bucket by un-xoring the retrieved
 * (bucket->zkey) value with the depth. */
struct tt_perft_bucket *get_bucket_lockless(u64 *zkey, u64 *depth)
{
	int i;
	struct tt_perft_bucket *bucket;
	bucket = first_bucket(zkey);
	for (i = 0; i < 4; i++, bucket++) {
		if ((((bucket->zkey) ^ *depth) == *zkey))
			return bucket;
	}
	return NULL;
}

void update_entry(u64 *zkey, u64 *nodes, int *depth)
{
	int i;
	struct tt_perft_bucket *bucket;
	bucket = first_bucket(zkey);
	for (i = 0; i < 4; i++, bucket++) {
		/* If the bucket is empty (since we used memset() earlier to set
		 * everything to zero, an easy way to do this is to check if the
		 * bucket's zkey is 0), store our info and exit.  Otherwise, see
		 * if the existing bucket's depth information is smaller than the
		 * one we're about to store, and if so, overwrite it.
		 */
		if ((bucket->zkey == 0x0ULL) || (*depth > get_depth(&bucket->info))) {
			u64 d = *depth;
			set_bucket_lockless(bucket, zkey, nodes, &d);
			tt.writes++;
			return;
		}
	}
}
