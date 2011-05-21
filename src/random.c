#include "random.h"

/* This is the KISS (MWC + CNG + XS) PRNG algorithm, devised by the late Dr.
 * George Marsaglia (1924-2011), creator of the "Diehard Battery of Test of
 * Randomness." Its main component is the MWC (multiply-with-carry) algorithm,
 * here in its 64-bit version, B64MWC(). Dr. Marsaglia devised this algorithm in
 * January 2011, when he was 87 years old, less than a month before his untimely
 * death. This PRNG's period is over 10^(40million), and passes all standard
 * randomness tests.  (See Dr. Marsaglia's post, "RNGs with periods exceeding
 * 10^(40million)." on sci.math for more information.)
 */

#define QSIZE 0x200000
#define CNG  ( cng=6906969069LL*cng+13579 )
#define XS ( xs^=(xs<<13), xs^=(xs>>17), xs^=(xs<<43) )
#define KISS ( B64MWC()+CNG+XS )

static u64 QARY[QSIZE];
static int j = QSIZE - 1;
static u64 carry = 0;
static u64 xs = 362436069362436069LL;
static u64 cng = 123456789987654321LL; /* use this as the seed */

u64 B64MWC()
{
	u64 t, x;
	j = (j + 1) & (QSIZE - 1);
	x = QARY[j];
	t = (x << 28) + carry;
	carry = (x >> 36) - (t < x);
	return (QARY[j] = t - x);
}

/* Initialize PRNG with default seed */
void randk_seed()
{
	u64 i;
	/* Seed QARY[] with CNG+XS: */
	for (i = 0; i < QSIZE; i++)
		QARY[i] = CNG + XS;
	/* Run through several rounds to warm up the state */
	for (i = 0; i < 104729; i++) /* 104729 is the 10,000th prime */
		randk();
}

void randk_seed_manual(u64 seed)
{
	cng = seed;
	randk_seed();
}

/* Generate a pseudorandom 64-bit unsigned integer. */
u64 randk()
{
	return KISS;
}
