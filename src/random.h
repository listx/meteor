#ifndef RANDOM_H
#define RANDOM_H

#include "types.h"

extern void randk_reset(void);
extern u64 B64MWC(void);
extern void randk_seed(void);
extern void randk_seed_manual(u64 seed);
extern void randk_warmup(int rounds);
extern u64 randk(void);

#endif
