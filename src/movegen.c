#include "movegen.h"

u64 attacks_table_R[102400];
int attacks_idx_R[64];
u64 mask_R[64];

const int shift_R[64] = {
        52, 53, 53, 53, 53, 53, 53, 52, 53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
        53, 54, 54, 54, 54, 54, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52
};

const u64 mult_R[64] = {
        0x0a8002c000108020ULL, 0x4440200140003000ULL, 0x8080200010011880ULL, 0x0380180080141000ULL,
        0x1a00060008211044ULL, 0x410001000a0c0008ULL, 0x9500060004008100ULL, 0x0100024284a20700ULL,
        0x0000802140008000ULL, 0x0080c01002a00840ULL, 0x0402004282011020ULL, 0x9862000820420050ULL,
        0x0001001448011100ULL, 0x6432800200800400ULL, 0x040100010002000cULL, 0x0002800d0010c080ULL,
        0x90c0008000803042ULL, 0x4010004000200041ULL, 0x0003010010200040ULL, 0x0a40828028001000ULL,
        0x0123010008000430ULL, 0x0024008004020080ULL, 0x0060040001104802ULL, 0x00582200028400d1ULL,
        0x4000802080044000ULL, 0x0408208200420308ULL, 0x0610038080102000ULL, 0x3601000900100020ULL,
        0x0000080080040180ULL, 0x00c2020080040080ULL, 0x0080084400100102ULL, 0x4022408200014401ULL,
        0x0040052040800082ULL, 0x0b08200280804000ULL, 0x008a80a008801000ULL, 0x4000480080801000ULL,
        0x0911808800801401ULL, 0x822a003002001894ULL, 0x401068091400108aULL, 0x000004a10a00004cULL,
        0x2000800640008024ULL, 0x1486408102020020ULL, 0x000100a000d50041ULL, 0x00810050020b0020ULL,
        0x0204000800808004ULL, 0x00020048100a000cULL, 0x0112000831020004ULL, 0x0009000040810002ULL,
        0x0440490200208200ULL, 0x8910401000200040ULL, 0x6404200050008480ULL, 0x4b824a2010010100ULL,
        0x04080801810c0080ULL, 0x00000400802a0080ULL, 0x8224080110026400ULL, 0x40002c4104088200ULL,
        0x01002100104a0282ULL, 0x1208400811048021ULL, 0x3201014a40d02001ULL, 0x0005100019200501ULL,
        0x0101000208001005ULL, 0x0002008450080702ULL, 0x001002080301d00cULL, 0x410201ce5c030092ULL
};

u64 attacks_table_X[5248];
int attacks_idx_X[64];
u64 mask_X[64];

const int shift_X[64] = {
        58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 59, 59,
        59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59,
        59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57, 57, 57, 59, 59,
        59, 59, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 58
};

const u64 mult_X[64] = {
        0x0440049104032280ULL, 0x1021023c82008040ULL, 0x0404040082000048ULL, 0x48c4440084048090ULL,
        0x2801104026490000ULL, 0x4100880442040800ULL, 0x0181011002e06040ULL, 0x9101004104200e00ULL,
        0x1240848848310401ULL, 0x2000142828050024ULL, 0x00001004024d5000ULL, 0x0102044400800200ULL,
        0x8108108820112000ULL, 0xa880818210c00046ULL, 0x4008008801082000ULL, 0x0060882404049400ULL,
        0x0104402004240810ULL, 0x000a002084250200ULL, 0x00100b0880801100ULL, 0x0004080201220101ULL,
        0x0044008080a00000ULL, 0x0000202200842000ULL, 0x5006004882d00808ULL, 0x0000200045080802ULL,
        0x0086100020200601ULL, 0xa802080a20112c02ULL, 0x0080411218080900ULL, 0x000200a0880080a0ULL,
        0x9a01010000104000ULL, 0x0028008003100080ULL, 0x0211021004480417ULL, 0x0401004188220806ULL,
        0x00825051400c2006ULL, 0x00140c0210943000ULL, 0x0000242800300080ULL, 0x00c2208120080200ULL,
        0x2430008200002200ULL, 0x1010100112008040ULL, 0x8141050100020842ULL, 0x0000822081014405ULL,
        0x800c049e40400804ULL, 0x4a0404028a000820ULL, 0x0022060201041200ULL, 0x0360904200840801ULL,
        0x0881a08208800400ULL, 0x0060202c00400420ULL, 0x1204440086061400ULL, 0x0008184042804040ULL,
        0x0064040315300400ULL, 0x0c01008801090a00ULL, 0x0808010401140c00ULL, 0x04004830c2020040ULL,
        0x0080005002020054ULL, 0x40000c14481a0490ULL, 0x0010500101042048ULL, 0x1010100200424000ULL,
        0x0000640901901040ULL, 0x00000a0201014840ULL, 0x00840082aa011002ULL, 0x010010840084240aULL,
        0x0420400810420608ULL, 0x8d40230408102100ULL, 0x4a00200612222409ULL, 0x0a08520292120600ULL
};

u64 attacks_pseudo_R[64];
u64 attacks_pseudo_X[64];
u64 attacks_pseudo_Q[64];

void init_attacks()
{
	int sq;
	int GO_R[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
	int GO_X[4][2] = {{1,1},{-1,1},{1,-1},{-1,-1}};
	init_attacks_sliding(attacks_table_R, attacks_idx_R, mask_R, shift_R, mult_R, GO_R);
	init_attacks_sliding(attacks_table_X, attacks_idx_X, mask_X, shift_X, mult_X, GO_X);
	for (sq = A1; sq != SQ_NONE; sq++) {
		attacks_pseudo_R[sq] = attacks_R(sq, 0x0ULL);
		attacks_pseudo_X[sq] = attacks_X(sq, 0x0ULL);
		attacks_pseudo_Q[sq] = attacks_Q(sq, 0x0ULL);
	}
}

void init_attacks_sliding(u64 attacks[], int attack_idx[], u64 mask[], const int shift[2], const u64 mult[], int delta[][2])
{
        int i, j, k, idx;
        u64 bb;

        idx = 0;
        for (i = 0; i < 64; i++) {
                attack_idx[i] = idx;
                mask[i] = sliding_attacks(i, 0x0ULL, 4, delta, 1, 6, 1, 6);
                j = (1 << (64 - shift[i]));
                for (k = 0; k < j; k++) {
                        bb = index_to_bitboard(k, mask[i]);
                        attacks[idx + ((bb * mult[i]) >> shift[i])] = sliding_attacks(i, bb, 4, delta, 0, 7, 0, 7);
                }
                idx += j;
        }
}

u64 sliding_attacks(int sq, u64 block, int dirs, int delta[][2], int fmin, int fmax, int rmin, int rmax)
{
        u64 result = 0x0ULL;
        int rk = sq / 8, fl = sq % 8, r, f, i;
        for (i = 0; i < dirs; i++) {
                int dx = delta[i][0], dy = delta[i][1];
                for (f = fl+dx, r = rk+dy; (dx==0 || (f>=fmin && f<=fmax)) && (dy==0 || (r>=rmin && r<=rmax)); f += dx, r += dy) {
			result |= (0x1ULL << (f + r*8));
			if (block & (0x1ULL << (f + r*8)))
				break;
                }
        }
        return result;
}

u64 index_to_bitboard(int idx, u64 mask)
{
        int a, b, bits;
        bits = bit_count(mask);
        u64 result = 0x0ULL;
        for (a = 0; a < bits; a++) {
                b = pop_LSB(&mask);
                if (idx & (1 << a))
			result |= (0x1ULL << b);
        }
        return result;
}

u8 movegen(struct position *pos, struct move *mlist, int us)
{
	u64 source, targets;
	u8 moves, moves_total;
	int sqk, sq1, sq2, pc;

	/*
	 * attacked	-> squares attacked (defended) by enemy pieces
	 * targets	-> enemy pieces that are caught w/in our moving piece's
	 *		   range
	 * pm		-> our moving piece type's bitboard
	 */

	find_pinned(pos);
	find_checkers(pos);

	sqk = sq_from_bit(&pos->piece[us][K]);
	sq1 = SQ_NONE;
	sq2 = SQ_NONE;
	pc = PIECE_NONE;
	moves = 0;

	moves_total  = movegen_N(pos, mlist, &moves, us, &targets, sqk, sq1, sq2, &source, pc);
	moves_total += movegen_K(pos, mlist + moves_total, &moves, us, &targets, sqk, sq2, pc);
	moves_total += movegen_P(pos, mlist + moves_total, &moves, us, &targets, sqk, sq1, sq2, &source, pc);
	moves_total += movegen_slider(pos, mlist + moves_total, &moves, us, &targets, sqk, sq1, sq2, &source, pc, R);
	moves_total += movegen_slider(pos, mlist + moves_total, &moves, us, &targets, sqk, sq1, sq2, &source, pc, X);
	moves_total += movegen_slider(pos, mlist + moves_total, &moves, us, &targets, sqk, sq1, sq2, &source, pc, Q);

	return moves_total;
}

u8 movegen_N(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq1, int sq2, u64 *source, int pc)
{
	int sq3;
	*moves = 0;

	/* Remove all pinned knights */
	*source = pos->piece[us][N] & ~pos->pinned;

	if (!pos->checkers) {
		while (*source) {
			sq1 = pop_LSB(source);
			*targets = MOVES_N[sq1] & ~pos->pieces[us];
			while (*targets) {
				sq2 = pop_LSB(targets);
				pc = pos->piece_on[sq2];
				mlist[(*moves)++].info = move_create_normal(sq1, sq2, N, pc);
			}
		}
	} else {
		/* only make Knight moves if there is only 1 checker; if there
		 * are 2 checkers, it is impossible to stop both of them with a
		 * single knight move */
		if (!(pos->checkers & (pos->checkers - 1))) {
			sq3 = sq_from_bit(&pos->checkers);
			while (*source) {
				sq1 = pop_LSB(source);
				*targets = MOVES_N[sq1] & pos->checkers;
				if (pos->checkers & sliders(!us, pos))
					*targets |= MOVES_N[sq1] & BITS_Q[sqk][sq3] & ~pos->occupied;
				while (*targets) {
					sq2 = pop_LSB(targets);
					pc = pos->piece_on[sq2];
					mlist[(*moves)++].info = move_create_normal(sq1, sq2, N, pc);
				}
			}
		}
	}
	return *moves;
}

u8 movegen_K(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq2, int pc)
{
	*moves = 0;

	*targets = MOVES_K[sqk] & ~pos->pieces[us] & ~bits_attacked_by(!us, pos);

	/* if the king is attacked by a sliding piece, then bits_attacked_by()
	 * does not return positive bits for those squares that are _behind_ the
	 * king (i.e., if enemy rook on e8, and king on e7, then e6 is an
	 * invalid square for the king, but is not covered by bits_attacked_by()
	 */
	/* FIXME: this check seems overly expensive... should be a simpler
	 * way... */
	if (pos->checkers) {
		u64 cs = pos->checkers;
		while (cs) {
			sq2 = pop_LSB(&cs);
			switch (pos->piece_on[sq2]) {
			case R: *targets &= ~attacks_R(sq2, pos->occupied & ~pos->piece[us][K]); break;
			case X: *targets &= ~attacks_X(sq2, pos->occupied & ~pos->piece[us][K]); break;
			case Q: *targets &= ~attacks_Q(sq2, pos->occupied & ~pos->piece[us][K]); break;
			default: break;
			}
		}
	}

	/* regular moves */
	while (*targets) {
		sq2 = pop_LSB(targets);
		pc = pos->piece_on[sq2];
		mlist[(*moves)++].info = move_create_normal(sqk, sq2, K, pc);
	}

	/* Castling moves */
	if (can_OO(us, &pos->info)) {
		/* Find partner rook */
		*targets = pos->piece[us][R] & BIT_FILE_RANK[birthfile_H(&pos->info)][rel_rank(us, RANK_1)];
		sq2 = sq_from_bit(targets);
		/* Find blockers */
		u64 blockers = pos->occupied & ~BIT[sqk] & ~BIT[sq2];
		/* Three tests for castling:
		 * - Ensure that thare are no pieces blocking the castling area
		 * - Ensure that that the squares the king is (1) currently on,
		 *   (2) will traverse, and (3) will end up on are not under
		 *   attack
		 * - Ensure that the partner rook is not pinned
		 */
                if (!((BITS_Q_INCL[sqk][rel_sq(us, G1)] | BITS_Q_INCL[sq2][rel_sq(us, F1)]) & blockers) &&
		    !(bits_attacked_by(!us, pos) & (BIT[sqk] | BITS_Q_INCL[sqk][sq_from(FILE_G, rel_rank(us, RANK_1))])) &&
		    !(BIT[sq2] & pos->pinned)
		    )
			mlist[(*moves)++].info = move_create(sqk, sq2, MOVE_OO, K, PIECE_NONE, PIECE_NONE);
	}
	if (can_OOO(us, &pos->info)) {
		*targets = pos->piece[us][R] & BIT_FILE_RANK[birthfile_A(&pos->info)][rel_rank(us, RANK_1)];
		sq2 = sq_from_bit(targets);
		u64 blockers = pos->occupied & ~BIT[sqk] & ~BIT[sq2];
                if (!((BITS_Q_INCL[sqk][rel_sq(us, C1)] | BITS_Q_INCL[sq2][rel_sq(us, D1)]) & blockers) &&
		    !(bits_attacked_by(!us, pos) & (BIT[sqk] | BITS_Q_INCL[sqk][sq_from(FILE_C, rel_rank(us, RANK_1))])) &&
		    !(BIT[sq2] & pos->pinned)
		    )
			mlist[(*moves)++].info = move_create(sqk, sq2, MOVE_OOO, K, PIECE_NONE, PIECE_NONE);
	}

	return *moves;
}

u8 movegen_P(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq1, int sq2, u64 *source, int pc)
{
        *moves = 0;
        *source = pos->piece[us][P];
        while (*source) {
                sq1 = pop_LSB(source);
                if (!pos->checkers) {
			/* Normal 1 or 2 square pawn pushes forward */

                        /* add a 1-square push move if 1 square in front of the pawn is empty */
                        *targets = ((BIT[sq1] << 8) >> (us << 4)) & ~pos->occupied;
                        if (*targets) {
                                sq2 = sq_from_bit(targets);
                                /* if this pawn is pinned, ignore pins that are
				 * on the same file */
                                if (BIT[sq1] & pos->pinned)
                                        *targets &= BIT_FILE[file_from(sqk)];
                                if (*targets) {
					/* regular 1-square pawn push forward */
                                        if (rank_from(sq1) != rel_rank(us, RANK_7)) {
                                                mlist[(*moves)++].info = move_create_normal(sq1, sq2, P, PIECE_NONE);
                                        } else {
						/* non-capture promotions */
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, PIECE_NONE, Q);
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, PIECE_NONE, R);
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, PIECE_NONE, X);
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, PIECE_NONE, N);
                                        }
                                }
				/* Double pawn pushes forward */

				/* Since we've already checked if there are no
				 * obstructions 1 rank above (3rd rank), we only
				 * need to check that (1) this pawn is on the
				 * 2nd rank, and (2) that the 4th rank for this
				 * file is empty
				 */
                                if (BIT[sq1] & BIT_RANK[rel_rank(us, RANK_2)]) {
					/* Since "*targets" is already 1 rank
					 * above, we only need to repeat the
					 * procedure we did above to set the
					 * rank to the 2nd rank above
					 */
                                        *targets = (((*targets << 8) >> (us << 4)) & ~pos->occupied);
                                        if (*targets) {
                                                sq2 = sq_from_bit(targets);
                                                if (BIT[sq1] & pos->pinned)
                                                        *targets &= BIT_FILE[file_from(sqk)];
                                                if (*targets) {
							/* check for EP
							 * opportunities
							 */
                                                        if (MOVES_K[sq2] & BIT_RANK[rel_rank(us, RANK_4)] & pos->piece[!us][P])
                                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_EP_CREATE, P, PIECE_NONE, PIECE_NONE);
                                                        else
                                                                mlist[(*moves)++].info = move_create_normal(sq1, sq2, P, PIECE_NONE);
                                                }
                                        }
                                }
                        }

			/* Normal captures */
                        *targets = ATTACKS_P[us][sq1] & pos->pieces[!us];

			/* If this pawn is pinned, it can make a capture of the
			 * pinner only
			 */
                        if (BIT[sq1] & pos->pinned) {
                                u64 pinner = attacks_Q(sqk, sliders(!us, pos)) & sliders(!us, pos);
                                while (pinner) {
                                        sq2 = pop_LSB(&pinner);
                                        if (BITS_Q[sqk][sq2] & BIT[sq1]) {
                                                *targets &= BIT[sq2];
                                                break;
                                        }
                                }
                        }
                        while (*targets) {
                                sq2 = pop_LSB(targets);
                                pc = pos->piece_on[sq2];
                                if (rank_from(sq2) != rel_rank(us, RANK_8)) {
                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_NORMAL, P, pc, PIECE_NONE);

                                } else {
				/* If the captured piece is on the 8th rank,
				 * then this is a promotion-capture
				 */
                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, Q);
                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, R);
                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, X);
                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, N);
                                }
                        }

			/* En passant captures */
                        if (ep_sq(&pos->info) != SQ_NONE) {
				/* internally, ep_sq is always on the 6th or 3rd
				 * rank */
				int eps = ep_sq(&pos->info);
				/* get rank of enemy pawn that just did a double
				 * pawn push
				 */
				int ep_pawn_sq = (eps + (us ? 8 : -8));
                                int epr = rank_from(ep_pawn_sq);
                                if (BIT[sq1] & BIT_RANK[epr] & MOVES_K[ep_pawn_sq]) {
					/* destination sq of ep capture is the
					 * sq _behind_ the ep square
					 */
                                        //sq2 = sq_from(epf, rel_rank(us, RANK_6));
					sq2 = ep_sq(&pos->info);
					/* Some chess laws:
					 *
					 * If the pawn is pinned, it can never
					 * make an en passant capture. (So we
					 * don't need to add any moves.)
					 *
					 * If the pawn is not pinned, it is
					 * still not allowed to make an en
					 * passant capture if it and the enemy
					 * EP pawn are the only two blockers
					 * between an enemy Rook or Queen and
					 * its own King (e.g., in FEN, something
					 * like: r1pP3K -> here, P cannot take
					 * the en-passant p, since that would
					 * result in: r6K, exposing the king.)
					 * Otherwise, the en passant capture is
					 * allowed.
					 */

                                        if (BIT[sq1] & ~pos->pinned) {
                                                u64 enemy_sliders_R = attacks_R(sqk, (pos->occupied & ~(BIT[sq1] | BIT[ep_pawn_sq]))) & sliders_R(!us, pos) & BIT_RANK[epr];
                                                if (BIT_RANK[epr] & BIT[sqk] && enemy_sliders_R) {
                                                        while (enemy_sliders_R) {
                                                                int sq3 = pop_LSB(&enemy_sliders_R);
                                                                u64 ray = BITS_R[sqk][sq3];
								if (!(ray & (BIT[sq1] | BIT[ep_pawn_sq]))) {
                                                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_EP_CAPTURE, P, P, PIECE_NONE);
                                                                        break;
                                                                }
                                                        }
                                                } else {
                                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_EP_CAPTURE, P, P, PIECE_NONE);
                                                }
                                        }
                                }
                        }
                } else {
			/*
			 * If we are in check, then any pawn that can move to
			 * thwart this checking threat has only 1 single valid
			 * move, unless that pawn's destination is the 8th rank
			 * (e.g., capturing a sliding piece that is on the 8th
			 * rank, or just pushing into the 8th rank to block an
			 * 8th rank ray).
			 *
			 * The basic way we handle this is to add all the valid
			 * target squares (sq2) first, and then by looking at
			 * the targte square, add in the appropriate moves at
			 * the end.
			 */

			/*
			 * Only try to add moves if there is 1 checker, and if
			 * this pawn is not pinned (pinned pawns cannot do
			 * anything if the king is in check).
			 */
                        if (pos->checkers && !(pos->checkers & (pos->checkers - 1)) && (BIT[sq1] & ~pos->pinned)) {
				/* Regular captures of checking piece */
                                *targets = ATTACKS_P[us][sq1] & pos->checkers;

                                /*
				 * En passant captures of checking piece
				 *
				 * If we are in check and the ep sq is not
				 * SQ_NONE, either (1) the ep pawn itself is the
				 * checking piece, OR (2) the ep pawn has moved
				 * out of the way of a sliding piece (which is
				 * the real checking piece); we can only add an
				 * ep capture move if it's the first condition.
				 */
                                if (ep_sq(&pos->info) != SQ_NONE) {
					int eps = ep_sq(&pos->info);
                                        int epf = file_from(eps);
					int ep_pawn_sq = (eps + (us ? 8 : -8));
                                        if ((BIT[ep_pawn_sq] & pos->checkers) &&
					    (BIT[sq1] & MOVES_K[ep_pawn_sq] & BIT_RANK[rel_rank(us, RANK_5)])) {
						*targets |= BIT_FILE_RANK[epf][rel_rank(us, RANK_6)];
					}
                                }
				/* Blocking moves for sliding checkers */
                                if (sliders(!us, pos) & pos->checkers) {
					/* get ray b/n our king and enemy
					 * sliding checker
					 */
                                        u64 ray = BITS_Q[sqk][sq_from_bit(&pos->checkers)];
					/* single pawn push */
                                        *targets |= ((BIT[sq1] << 8) >> (us << 4)) & ~pos->occupied & ray;
                                        /* double pawn push */
                                        u64 sqfront = ((BIT[sq1] << 8) >> (us << 4)) & ~pos->occupied;
                                        sqfront = ((sqfront << 8) >> (us << 4)) & ~pos->occupied;
                                        if (sqfront && (BIT[sq1] & BIT_RANK[rel_rank(us, RANK_2)]))
                                                *targets |= sqfront & ray;
                                }
				/* Add correct moves based on target squares */
                                while (*targets) {
                                        sq2 = pop_LSB(targets);
                                        pc = pos->piece_on[sq2];

					/* If an ep pawn is checking, it can be
					 * either an ep capture or a regular
					 * capture
					 */
                                        if (ep_sq(&pos->info) != SQ_NONE) {
						/* If sq2 is the same square as
						 * the ep pawn, then it's a
						 * regular capture
						 */
                                                if (BIT[sq2] & pos->checkers)
                                                        mlist[(*moves)++].info = move_create_normal(sq1, sq2, P, P);
                                                else
                                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_EP_CAPTURE, P, P, PIECE_NONE);
						/* Since a pawn doing an ep
						 * capture _while in check_ can
						 * do no other moves, break loop
						 * for this pawn to save time.
						 */
                                                break;
                                        }

					/* normal captures and single/double
					 * pawn pushes, and promotions
					 */
                                        if (rank_from(sq2) != rel_rank(us, RANK_8)) {
						/* check for EP creation */
                                                if ((dist_sq(sq1, sq2) > 1) && (((ATTACKS_P[us][sq1] << 8) >> (us << 4)) & pos->piece[!us][P]))
                                                        mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_EP_CREATE, P, pc, PIECE_NONE);
                                                else
                                                        mlist[(*moves)++].info = move_create_normal(sq1, sq2, P, pc);
                                        } else {
						/* promotions */
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, Q);
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, R);
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, X);
                                                mlist[(*moves)++].info = move_create(sq1, sq2, MOVE_PROM, P, pc, N);
                                        }
                                }
                        }
                }
        }
	return *moves;
}

u8 movegen_slider(struct position *pos, struct move *mlist, u8 *moves, int us, u64 *targets, int sqk, int sq1, int sq2, u64 *source, int pc, int piece_slider)
{
        *moves = 0;

        if (!pos->checkers) {
                *source = pos->piece[us][piece_slider];
                while (*source) {
                        sq1 = pop_LSB(source);
			switch (piece_slider) {
			case R:
				*targets = attacks_R(sq1, pos->occupied) & ~pos->pieces[us];
				break;
			case X:
				*targets = attacks_X(sq1, pos->occupied) & ~pos->pieces[us];
				break;
			case Q:
				*targets = attacks_Q(sq1, pos->occupied) & ~pos->pieces[us];
				break;
			default:
				assert(0);
			}
                        if (BIT[sq1] & pos->pinned) {
                                // if it's pinned, we can only try to capture the pinner, or move along the pinned ray
                                // determine who is pinning us, among the numerous other possible pinners out there
                                u64 pinner = attacks_Q(sqk, sliders(!us, pos)) & sliders(!us, pos);
                                while (pinner) {
                                        sq2 = pop_LSB(&pinner);
                                        if (BITS_Q[sqk][sq2] & BIT[sq1]) {
                                                switch (pos->piece_on[sq2]) {
						case R: *targets &= BITS_R[sqk][sq2] | BIT[sq2]; break;
						case X: *targets &= BITS_X[sqk][sq2] | BIT[sq2]; break;
						case Q: *targets &= BITS_Q[sqk][sq2] | BIT[sq2]; break;
						default: break;
                                                }
						/* quit the search once we find
						 * our pinner */
                                                break;
                                        }
                                }
                        }
                        while(*targets) {
                                sq2 = pop_LSB(targets);
                                pc = pos->piece_on[sq2];
                                mlist[(*moves)++].info = move_create_normal(sq1, sq2, piece_slider, pc);
                        }
                }
        } else {
		/* if there is a single checker (in face of double check, only
		 * the king may move) */
                if (!(pos->checkers & (pos->checkers - 1))) {
                        u64 attacks;
                        int sq3 = sq_from_bit(&pos->checkers);
                        *source = pos->piece[us][piece_slider] & ~pos->pinned;
                        while (*source) {
                                sq1 = pop_LSB(source);
				switch (piece_slider) {
				case R:
					attacks = attacks_R(sq1, pos->occupied);
					break;
				case X:
					attacks = attacks_X(sq1, pos->occupied);
					break;
				case Q:
					attacks = attacks_Q(sq1, pos->occupied);
					break;
				default:
					assert(0);
				}
                                *targets = attacks & pos->checkers;
                                if (pos->checkers & sliders(!us, pos))
                                        *targets |= attacks & BITS_Q[sqk][sq3];
                                while (*targets) {
                                        sq2 = pop_LSB(targets);
                                        pc = pos->piece_on[sq2];
                                        mlist[(*moves)++].info = move_create_normal(sq1, sq2, piece_slider, pc);
                                }
                        }
                }
        }
        return *moves;
}

void move_do(struct position *pos, u32 *move_info, u32 *undo_info)
{
	int us, sq1, sq2, sq3, sq4, pm, pc, mt;

	*undo_info = 0x0;
	/* grab the lowest 32 bits of pos->info, because it holds data on
	 * everything we want */
	*undo_info |= pos->info;
	us = our_color(&pos->info);
	sq1 = move_sq1(move_info);
	sq2 = move_sq2(move_info);
	pm = mpiece(move_info);
	pc = cpiece(move_info);
	mt = move_type(move_info);

	/* take care of sq1 */
	pos->piece[us][pm] &= ~BIT[sq1];
	pos->pieces[us] &= ~BIT[sq1];
	pos->occupied &= ~BIT[sq1];
	pos->piece_on[sq1] = PIECE_NONE;

	switch (mt) {
	case MOVE_NORMAL:
		/*
		 * If king has moved or rook has moved, then revoke appropriate
		 * castling rights.
		 */

		if (can_OO_or_OOO(us, &pos->info)) {
			if (pm == K)
				revoke_OO_OOO(us, &pos->info);
			else if (pm == R)
				set_virgin_rook_changed(us, sq1, &pos->info);
		}

		set_ep_sq(SQ_NONE, &pos->info);
		pos->piece[us][pm] |= BIT[sq2];
		pos->pieces[us] |= BIT[sq2];
		pos->occupied |= BIT[sq2];
		pos->piece_on[sq2] = pm;

		if (pc == PIECE_NONE) {
			if (pm == P)
				reset_FMR(&pos->info);
			else
				inc_FMR(&pos->info);
		} else {
			pos->piece[!us][pc] &= ~BIT[sq2];
			pos->pieces[!us] &= ~BIT[sq2];
			/* don't (pos->occupied &= ~BIT[sq2]) because sq2 is
			 * occupied by our piece that just moved there
			 */
			reset_FMR(&pos->info);
			/* revoke enemy OO or OOO if cpiece is virgin rook */
			if (pc == R)
				set_virgin_rook_changed(!us, sq2, &pos->info);
		}
		break;
	case MOVE_OO:
	case MOVE_OOO:
		/* Place king on destination square */
                sq3 = sq_from(((mt == MOVE_OO) ? FILE_G : FILE_C), rel_rank(us, RANK_1));
                pos->piece[us][K] = BIT[sq3];
                pos->pieces[us] |= BIT[sq3];
                pos->occupied |= BIT[sq3];
                pos->piece_on[sq3] = K;

                /* Take care of rook's new square */
                sq4 = sq_from(((mt == MOVE_OO) ? FILE_F : FILE_D), rel_rank(us, RANK_1));
		/* We stores castling moves as "king moves to partner rook's
		 * square", so we need to clear sq2 for the rook's bitboard
		 */
                pos->piece[us][R] &= ~BIT[sq2];
                pos->pieces[us] &= ~BIT[sq2];
                pos->occupied &= ~BIT[sq2];
		/* declare that rook's starting square (sq2) is empty only if if
		 * king's destination square is the same as sq2
		 */
                if (sq2 != sq3) {
                        pos->piece_on[sq2] = PIECE_NONE;
                }
                /* Ensure that the rook ends up on either F1 (if O-O) or C1 (if
		 * O-O-O)
		 */
                pos->piece[us][R] |= BIT[sq4];
                pos->pieces[us] |= BIT[sq4];
                pos->occupied |= BIT[sq4];
                pos->piece_on[sq4] = R;
                /* Castling once means no more castling rights */
                revoke_OO_OOO(us, &pos->info);
                set_ep_sq(SQ_NONE, &pos->info);
                inc_FMR(&pos->info);
		break;
	case MOVE_EP_CREATE:
                pos->piece[us][P] |= BIT[sq2];
                pos->pieces[us] |= BIT[sq2];
                pos->occupied |= BIT[sq2];
                pos->piece_on[sq2] = P;
                set_ep_sq(sq2 + (us ? 8 : -8), &pos->info);
                reset_FMR(&pos->info);
		break;
	case MOVE_EP_CAPTURE:
                sq3 = sq_from(file_from(sq2), rel_rank(us, RANK_5));
		/* clear captured pawn's square */
                pos->piece[!us][P] &= ~BIT[sq3];
                pos->pieces[!us] &= ~BIT[sq3];
                pos->occupied &= ~BIT[sq3];
                pos->piece_on[sq3] = PIECE_NONE;

                set_ep_sq(SQ_NONE, &pos->info);
                pos->piece[us][P] |= BIT[sq2];
                pos->pieces[us] |= BIT[sq2];
                pos->occupied |= BIT[sq2];
                pos->piece_on[sq2] = P;
                reset_FMR(&pos->info);
                break;
	case MOVE_PROM:
                set_ep_sq(SQ_NONE, &pos->info);
                pos->piece[us][ppiece(move_info)] |= BIT[sq2];
                pos->pieces[us] |= BIT[sq2];
                pos->occupied |= BIT[sq2];
                pos->piece_on[sq2] = ppiece(move_info);
                reset_FMR(&pos->info);
                if (pc != PIECE_NONE) {
                        pos->piece[!us][pc] &= ~BIT[sq2];
                        pos->pieces[!us] &= ~BIT[sq2];
			/* if pc is rook, check if it was a virgin rook and
			 * revoke appropriate castling right
			 */
                        if (pc == R)
				set_virgin_rook_changed(!us, sq2, &pos->info);
                }
		break;
	default:
		break;
	}

	swap_turn(&pos->info);
}

void move_undo(struct position *pos, u32 *move_info, u32 *undo_info)
{
	int us, sq1, sq2, sq3, sq4, r, pm, pc, mt;

	swap_turn(&pos->info);
	/* clear bottom 32 bits of pos->info */
	pos->info &= 0xffffffff00000000ULL;
	/* restore bottom 32 bits of old pos->info */
	pos->info |= *undo_info;

	us = our_color(&pos->info);
	sq1 = move_sq1(move_info);
	sq2 = move_sq2(move_info);
	sq3 = SQ_NONE;
	sq4 = SQ_NONE;
	pm = mpiece(move_info);
	pc = cpiece(move_info);
	mt = move_type(move_info);

	/* restore sq1 */
	pos->piece[us][pm] |= BIT[sq1];
	pos->pieces[us] |= BIT[sq1];
	pos->occupied |= BIT[sq1];
	pos->piece_on[sq1] = pm;

	switch (mt) {
	case MOVE_NORMAL:
	case MOVE_EP_CREATE:
	case MOVE_PROM:
		/* remove own piece from sq2 */
		pos->piece[us][pm] &= ~BIT[sq2];
		pos->pieces[us] &= ~BIT[sq2];
		pos->occupied &= ~BIT[sq2];

		if (pc == PIECE_NONE) {
			pos->piece_on[sq2] = PIECE_NONE;
		} else {
			/* resurrect enemy piece */
			pos->piece[!us][pc] |= BIT[sq2];
			pos->pieces[!us] |= BIT[sq2];
			pos->occupied |= BIT[sq2];
			pos->piece_on[sq2] = pc;
		}

		/* if move was a promotion, then banish the promoted-to piece */
		if (mt == MOVE_PROM)
			pos->piece[us][ppiece(move_info)] &= ~BIT[sq2];
		break;
	case MOVE_EP_CAPTURE:
		/* resurrect the enemy pawn */
		sq3 = sq_from(file_from(sq2), rel_rank(us, RANK_5));
		pos->piece[!us][P] |= BIT[sq3];
		pos->pieces[!us] |= BIT[sq3];
		pos->occupied |= BIT[sq3];
		pos->piece_on[sq3] = P;
		/* s2 must be cleared, since a pawn in an ep capture _moves_ to
		 * an empty square (and captures from a different square)
		 */
		pos->piece[us][P] &= ~BIT[sq2];
		pos->pieces[us] &= ~BIT[sq2];
		pos->occupied &= ~BIT[sq2];
		pos->piece_on[sq2] = PIECE_NONE;
		break;
	case MOVE_OO:
	case MOVE_OOO:
		/*
		 * First, remove a rook from its castled-to position.
		 * Second, remove the king from its castled-to position (but
		 * only if its previous square was NOT the same as its current
		 * position).
		 */

		r = rel_rank(us, RANK_1);
		if (mt == MOVE_OO) {
			sq3 = sq_from(FILE_G, r);
			sq4 = sq_from(FILE_F, r);
		} else {
			sq3 = sq_from(FILE_C, r);
			sq4 = sq_from(FILE_D, r);
		}
		/*
		 * If the king moved while castling, then since we've already
		 * restored the king on sq1 already, we need to get rid of the
		 * king on sq3
		 */
		if (sq3 != sq1) {
			pos->piece[us][K] &= ~BIT[sq3];
			pos->pieces[us] &= ~BIT[sq3];
			pos->occupied &= ~BIT[sq3];
			pos->piece_on[sq3] = PIECE_NONE;
		}
		pos->piece[us][R] &= ~BIT[sq4];
		pos->pieces[us] &= ~BIT[sq4];
		pos->occupied &= ~BIT[sq4];
		/*
		 * It's possible now that sq1 == sq4, in which case, we can only
		 * allow pos->piece_on[sq4] == PIECE_NONE if sq1 != sq4.
		 */
		if (sq4 != sq1) {
			pos->piece_on[sq4] = PIECE_NONE;
		}

		/*
		 * Place new rook back into its old position; we can use sq2,
		 * b/c we store castling moves as "K moves to R's square".
		 */
		pos->piece[us][R] |= BIT[sq2];
		pos->pieces[us] |= BIT[sq2];
		pos->occupied |= BIT[sq2];
		pos->piece_on[sq2] = R;
		break;
	default: break;
	}
}
