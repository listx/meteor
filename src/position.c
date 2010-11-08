#include "position.h"

u64 MOVES_K[64];
u64 MOVES_Q[64];
u64 MOVES_R[64];
u64 MOVES_X[64];

u64 BIT_TURN		= 0x1ULL << SHF_TURN;
u64 BIT_IN_CHECK	= 0x1ULL << SHF_IN_CHECK;
u64 BIT_OO_W		= 0x1ULL << SHF_OO_W;
u64 BIT_OOO_W		= 0x1ULL << SHF_OOO_W;
u64 BIT_OO_B		= 0x1ULL << SHF_OO_B;
u64 BIT_OOO_B		= 0x1ULL << SHF_OOO_B;
u64 BITS_HROOK_FILE	= 0x7ULL << SHF_HROOK_FILE;
u64 BITS_AROOK_FILE	= 0x7ULL << SHF_AROOK_FILE;
u64 BITS_EP_SQ		= 0x7fULL << SHF_EP_SQ;
u64 BITS_FMR		= 0x7fULL << SHF_FMR;

/*
 * Import a Shredder-FEN string into the internal board position representation.
 */
void import_sfen(const char *str, struct position *pos)
{
	int rank, file, piece, sq, sq_cnt, idx, ep_pawn_rank, fmr;
	unsigned int i, j;
	unsigned int bytes;
	char *tokenize_me;
	char *token;
	u64 rook, ep_pawn_adjacent;

	pos_clear(pos);

	/* Sample Shredder-FEN:
	 * nrnkbbqr/pppppppp/8/8/8/8/PPPPPPPP/NRNKBBQR w BbHh - 0 1
	 * - pieces
	 * - side to move
	 * - castling rights (if no rights, just "-")
	 * - square behind double pawn push'ed square (e.g., e3 after the first
	 *   move)
	 * - fifty move rule counter (plies -- 100 denotes FMR draw; default 0)
	 * - move counter (full moves, not plies; default 1)
	 */

	/* Prepare tokenization */
	bytes = sizeof(*tokenize_me) * (strlen(str) + 1);
	tokenize_me = malloc(bytes);
	strncpy(tokenize_me, str, bytes);

	/* Ensure that there are 5 space characters in the string (for proper
	 * tokenization */
	for (i = 0, j = 0; i + 1 < strlen(str); i++) {
		if (isspace(str[i]) && !isspace(str[i+1]))
			j++;
	}
	if (j != 5)
		fatal("Invalid number of fields");

	/* Piece placement */
	token = strtok(tokenize_me, " ");
	piece = PIECE_NONE;
	rank = RANK_8;
	file = FILE_A;
	sq_cnt = 0;	/* Running total of squares accounted for */

	for (idx = 0; token[idx]; idx++, piece = PIECE_NONE) {
		assert(valid_rank(rank));
		assert(valid_file(file));
		switch (token[idx]) {
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			piece = PIECE_NONE;
			file += (token[idx] - '1') + 1;
			sq_cnt += (token[idx] - '1') + 1;
			break;
		case 'K': piece = WK; break;
		case 'Q': piece = WQ; break;
		case 'R': piece = WR; break;
		case 'B': piece = WX; break;
		case 'N': piece = WN; break;
		case 'P': piece = WP; break;
		case 'k': piece = BK; break;
		case 'q': piece = BQ; break;
		case 'r': piece = BR; break;
		case 'b': piece = BX; break;
		case 'n': piece = BN; break;
		case 'p': piece = BP; break;
		case '/': piece = PIECE_NONE; break;
		default:
			fatal("Invalid character `%c' in piece placement info", token[idx]);
		}
		if (piece != PIECE_NONE) {
			pos->piece[piece] |= BIT[((rank * 8) + file)];
			file++;
			sq_cnt++;
		}
		if (file > FILE_H) {
			rank--;
			file = FILE_A;
		}
	}
	/* Populate pos->piece_on[] array */
	for (sq = A1; sq != SQ_NONE; sq++) {
		for (piece = WK; piece != PIECE_NONE; piece++) {
			if (BIT[sq] & pos->piece[piece]) {
				pos->piece_on[sq] = piece;
				break;
			}
		}
	}
	/* Has every square on the chessboard been accounted for? */
	assert(sq_cnt == 64);

	/* Some more tests to verify validity of FEN */

	/* There must be 1 king present for each color */
	if (pos->piece[WK] == 0x0ULL)
		fatal("White king missing");
	if (pos->piece[BK] == 0x0ULL)
		fatal("Black king missing");
	/* Kings may not be adjacent to each other */
	if (MOVES_K[sq_from_bit(&pos->piece[WK])] & pos->piece[BK])
		fatal("Kings are adjacent");

	/* Side to move (0 is White, 1 is Black) */
	idx = 0;
	token = strtok(NULL, " ");
	assert(token[idx] == 'w' || token[idx] == 'b');
	if (token[idx] == 'w')
		pos->info &= ~BIT_TURN;
	else
		pos->info |= BIT_TURN;

	/* Castling rights */

	token = strtok(NULL, " ");
	if (strlen(token) > 4)
		fatal("castling right information exceeded 4 characters");
	if (token[0] != '-') {
		for (i = 0; i < strlen(token); i++) {
			switch (token[i]) {
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
				file = token[i] - 'A';
				/* Isolate the rook that is on the RIGHT side of
				 * the king */
				rook = pos->piece[WR] & BIT_FILE_RANK[file][RANK_1];
				if (rook) {
					pos->info |= ((u64)file) << SHF_HROOK_FILE;
					if (sq_from_bit(&rook) > sq_from_bit(&pos->piece[WK]))
						pos->info |= BIT_OO_W;
					else
						pos->info |= BIT_OOO_W;
				}
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
				file = token[i] - 'a';
				rook = pos->piece[BR] & BIT_FILE_RANK[file][RANK_8];
				if (rook) {
					pos->info |= ((u64)file) << SHF_HROOK_FILE;
					if (sq_from_bit(&rook) > sq_from_bit(&pos->piece[BK]))
						pos->info |= BIT_OO_B;
					else
						pos->info |= BIT_OOO_B;
				}
				break;
			default:
				fatal("Invalid castling file `%c'", token[i]);
			}
		}
	}

	/* En passant square (only acknowledge actual en passant opportunities) */
	token = strtok(NULL, " ");
	if (strlen(token) > 2) {
		fatal("En passant square field `%s' exceeds 2 characters", token);
	}
	if (token[0] != '-') {
		switch (token[0]) {
		case 'a': file = FILE_A; break;
		case 'b': file = FILE_B; break;
		case 'c': file = FILE_C; break;
		case 'd': file = FILE_D; break;
		case 'e': file = FILE_E; break;
		case 'f': file = FILE_F; break;
		case 'g': file = FILE_G; break;
		case 'h': file = FILE_H; break;
		default:
			fatal("Invalid character `%c'", token[0]);
		}
		switch (token[1]) {
		case '3': rank = RANK_3; break;
		case '6': rank = RANK_6; break;
		default:
			fatal("Invalid character `%c'", token[0]);
		}
		/* Check if we can in fact make an EP capture */
		ep_pawn_rank = (rank == RANK_3) ? RANK_4 : RANK_5;
		sq = sq_from(file, ep_pawn_rank);
		ep_pawn_adjacent = MOVES_K[sq] & BIT_RANK[ep_pawn_rank];
		if (ep_pawn_adjacent & pos->piece[(pos->info & BIT_TURN) ? BP : WP]) {
			pos->info &= ~BITS_EP_SQ;
			pos->info |= ((u64)sq_from(file, rank)) << SHF_EP_SQ;
		}
	}

	/* Fifty move clock (0 to 100 plies) */
	token = strtok(NULL, " ");
	fmr = atoi(token);
	pos->info |= ((u64)fmr) << SHF_FMR;

	/*
	 * Whole move clock
	 *
	 * Since this info is not essential, we do not store it in the pos->info
	 * bitmap. For now, we ignore it.
	 */
}

void pos_clear(struct position *pos)
{
	int i;

	for (i = 0; i != PIECE_NONE; i++) {
		pos->piece[i] = 0x0ULL;
	}

	pos->pieces[W]	= 0x0ULL;
	pos->pieces[B]	= 0x0ULL;
	pos->occupied	= 0x0ULL;
	pos->pinned	= 0x0ULL;
	pos->checkers	= 0x0ULL;
	pos->info	= 0x0ULL;
	/*
	 * Castling rights
	 *
	 * Since pos->info is zeroed out, both its short and long castling files
	 * are set to FILE_A; thus, if there are no castling rights read from
	 * the Shredder-FEN, we will be left with pos->info declaring that both
	 * short and long sides are OK to be castled from FILE_A. To prevent
	 * this glitch, we manually set the long side (A-side) castling file to
	 * FILE_H, since the code that performs castling will never evaluate
	 * FILE_H for that side.
	 *
	 * In short, we properly set a fake NULL value of FILE_H into the A-side
	 * rook's starting file. This way, even if the Shredder-FEN does not
	 * have any castling information, the proper NULL values prevent Meteor
	 * from undertaking any spurious castling moves.
	 */
	pos->info |= ((u64)FILE_H) << SHF_AROOK_FILE;
	/* Set En Passant sq to NONE */
	pos->info |= ((u64)SQ_NONE) << SHF_EP_SQ;
	pos->zkey	= 0x0ULL;

	for (i = 0; i != SQ_NONE; i++) {
		pos->piece_on[i] = PIECE_NONE;
	}
}

/* Display the board to STDOUT, with colors */
void disp_pos(struct position *pos)
{
	int rank, sq, i;
	char pieces[9];
	char sq_str[3];
	sq_str[2] = '\0';
	printf("    a   b   c   d   e   f   g   h\n");
	printf("  \e[0;34m+---+---+---+---+---+---+---+---+\e[0m\n");
	for (rank = RANK_8; rank >= RANK_1; rank--) {
		set_pieces_rank(rank, pos, pieces);
		printf("%d ", rank + 1);
		printf("\e[0;34m|\e[0m");
		for (i = 0; i < 8; i++) {
			/* Checkerboard pattern */
			if (((((rank * 8) + i + 1) + ((rank % 2))) % 2) == 0)
				printf("\e[44m");
			printf(" ");
			switch (pieces[i]) {
			case 'K':
			case 'Q':
			case 'R':
			case 'B':
			case 'N':
				printf("\e[1;37m%c", pieces[i]);
				break;
			case 'P':
				printf("\e[1;37m*");
				break;
			case 'k':
			case 'q':
			case 'r':
			case 'b':
			case 'n':
				/* Uppercase it back */
				printf("\e[1;31m%c", pieces[i] - 0x20);
				break;
			case 'p':
				printf("\e[1;31m*");
				break;
			case ' ':
				printf(" ");
				break;
			default:
				assert(0);
			}
			printf(" \e[0m\e[0;34m|\e[0m");
		}
		printf(" %d\n", rank + 1);
		printf("  \e[0;34m+---+---+---+---+---+---+---+---+\e[0m\n");
	}
	printf("    a   b   c   d   e   f   g   h\n");

	/* Side to move */
	printf("Side to move: %s\n", (pos->info & BIT_TURN) ? "Black" : "White");
	/* Castling rights */
	printf("Castling rights:\n");
	printf("  White: ");
	if (pos->info & BIT_OO_W) {
		printf("O-O");
		if (!(pos->info & BIT_OOO_W))
			printf("\n");
	}
	if (pos->info & BIT_OOO_W) {
		if (pos->info & BIT_OO_W)
			printf(", ");
		printf("O-O-O\n");
	}
	printf("  Black: ");
	if (pos->info & BIT_OO_B) {
		printf("O-O");
		if (!(pos->info & BIT_OOO_B))
			printf("\n");
	}
	if (pos->info & BIT_OOO_B) {
		if (pos->info & BIT_OO_B)
			printf(", ");
		printf("O-O-O\n");
	}
	/* En passant (true EP shown only, not "EP" square from FEN) */
	if (pos->info & BITS_EP_SQ) {
		sq = (pos->info & BITS_EP_SQ) >> SHF_EP_SQ;
		printf("En Passant Square: ");
		if (sq != SQ_NONE)
			printf("%s\n", sq_to_str(sq, sq_str));
		else
			printf("None\n");
	}
}

/* For debugging */
void disp_bitboard(u64 bb)
{
	int rank, file, sq;
	for (rank = RANK_8; rank >= RANK_1; rank--) {
		for (file = FILE_A; file <= FILE_H; file++) {
			sq = (rank * 8) + file;
			assert(BIT[sq]);
			if (BIT[sq] & bb)
				printf("O");
			else
				printf(".");
		}
		printf("\n");
	}
}

/*
 * Set an 8-character string that encodes the info of the pieces on the
 * position at a given rank.
 */
void set_pieces_rank(int rank, struct position *pos, char *pieces)
{
	int piece, sq, idx;

	idx = 0;

	for (sq = rank * 8; sq < ((rank * 8) + 8); sq++, idx++) {
		pieces[idx] = ' ';
		for (piece = WK; piece != PIECE_NONE; piece++) {
			if (BIT[sq] & pos->piece[piece]) {
				switch (piece) {
				case WK: pieces[idx] = 'K'; break;
				case WQ: pieces[idx] = 'Q'; break;
				case WR: pieces[idx] = 'R'; break;
				case WX: pieces[idx] = 'B'; break;
				case WN: pieces[idx] = 'N'; break;
				case WP: pieces[idx] = 'P'; break;
				case BK: pieces[idx] = 'k'; break;
				case BQ: pieces[idx] = 'q'; break;
				case BR: pieces[idx] = 'r'; break;
				case BX: pieces[idx] = 'b'; break;
				case BN: pieces[idx] = 'n'; break;
				case BP: pieces[idx] = 'p'; break;
				default: assert(0);
				}
				break;
			}
		}
	}
	pieces[8] = '\0';
}
