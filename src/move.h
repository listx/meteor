#ifndef MOVE_H
#define  MOVE_H

#include "io.h"
#include "types.h"

extern u64 MOVES_K[64];
extern u64 MOVES_Q[64];
extern u64 MOVES_R[64];
extern u64 MOVES_X[64];

extern void init_bitmasks_moves();

#endif
