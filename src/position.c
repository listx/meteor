#include "position.h"

u64 MOVES_K[64];
u64 MOVES_Q[64];
u64 MOVES_R[64];
u64 MOVES_X[64];

u64 BIT_TURN		= 0x1ULL << SHF_TURN;
u64 BIT_OO_W		= 0x1ULL << SHF_OO_W;
u64 BIT_OOO_W		= 0x1ULL << SHF_OOO_W;
u64 BIT_OO_B		= 0x1ULL << SHF_OO_B;
u64 BIT_OOO_B		= 0x1ULL << SHF_OOO_B;
u64 BITS_HROOK_FILE	= 0x7ULL << SHF_HROOK_FILE;
u64 BITS_AROOK_FILE	= 0x7ULL << SHF_AROOK_FILE;
u64 BITS_EP_SQ		= 0x7fULL << SHF_EP_SQ;
u64 BITS_FMR		= 0x7fULL << SHF_FMR;

u64 BITS_CASR		= 0xfULL << SHF_CASR;

/*
 * Import a Shredder-FEN string into the internal board position representation.
 */
void import_sfen(const char *str, struct position *pos)
{
	int rank, file, piece, color, sq, sq_cnt, idx, ep_pawn_rank, fmr;
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
		fatal("sfen: Invalid number of fields");

	/* Piece placement */
	token = strtok(tokenize_me, " ");
	piece = PIECE_NONE;
	color = COLOR_NONE;
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
		case 'K': color = W; piece = K; break;
		case 'Q': color = W; piece = Q; break;
		case 'R': color = W; piece = R; break;
		case 'B': color = W; piece = X; break;
		case 'N': color = W; piece = N; break;
		case 'P': color = W; piece = P; break;
		case 'k': color = B; piece = K; break;
		case 'q': color = B; piece = Q; break;
		case 'r': color = B; piece = R; break;
		case 'b': color = B; piece = X; break;
		case 'n': color = B; piece = N; break;
		case 'p': color = B; piece = P; break;
		case '/': piece = PIECE_NONE; break;
		default:
			fatal("sfen: Invalid character `%c' in piece placement info", token[idx]);
		}
		if (piece != PIECE_NONE) {
			pos->piece[color][piece] |= BIT[((rank * 8) + file)];
			file++;
			sq_cnt++;
		}
		if (file > FILE_H) {
			rank--;
			file = FILE_A;
		}
	}
	/* Populate misc information re: piece placement */
	for (sq = A1; sq != SQ_NONE; sq++) {
		for (color = W; color != COLOR_NONE; color++) {
			for (piece = K; piece != PIECE_NONE; piece++) {
				if (BIT[sq] & pos->piece[color][piece]) {
					pos->piece_on[sq] = piece;
					pos->pieces[color] |= BIT[sq];
					pos->occupied |= BIT[sq];
					break;
				}
			}
		}
	}
	/* Has every square on the chessboard been accounted for? */
	assert(sq_cnt == 64);

	/* Some more tests to verify validity of FEN */

	/* There must be 1 king present for each color */
	if (pos->piece[W][K] == 0x0ULL)
		fatal("sfen: White king missing");
	if (pos->piece[B][K] == 0x0ULL)
		fatal("sfen: Black king missing");
	/* Kings may not be adjacent to each other */
	if (MOVES_K[sq_from_bit(&pos->piece[W][K])] & pos->piece[B][K])
		fatal("sfen: Kings are adjacent");

	/* Side to move (0 is White, 1 is Black) */
	idx = 0;
	token = strtok(NULL, " ");
	assert(token[idx] == 'w' || token[idx] == 'b');
	if (token[idx] == 'w')
		pos->info &= ~BIT_TURN;
	else
		pos->info |= BIT_TURN;

	/* Set auxiliary piece bitboards */
	pos->pieces[W] =
		pos->piece[W][K] |
		pos->piece[W][Q] |
		pos->piece[W][R] |
		pos->piece[W][X] |
		pos->piece[W][N] |
		pos->piece[W][P];
	pos->pieces[B] =
		pos->piece[B][K] |
		pos->piece[B][Q] |
		pos->piece[B][R] |
		pos->piece[B][X] |
		pos->piece[B][N] |
		pos->piece[B][P];
	pos->occupied =
		pos->pieces[W] | pos->pieces[B];

	/* Castling rights */

	token = strtok(NULL, " ");
	if (strlen(token) > 4)
		fatal("sfen: castling right information exceeded 4 characters");
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
				rook = pos->piece[W][R] & BIT_FILE_RANK[file][RANK_1];
				if (rook) {
					if (sq_from_bit(&rook) > sq_from_bit(&pos->piece[W][K])) {
						set_birthfile_H(file, &pos->info);
						pos->info |= BIT_OO_W;
					} else {
						set_birthfile_A(file, &pos->info);
						pos->info |= BIT_OOO_W;
					}
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
				rook = pos->piece[B][R] & BIT_FILE_RANK[file][RANK_8];
				if (rook) {
					if (sq_from_bit(&rook) > sq_from_bit(&pos->piece[B][K])) {
						set_birthfile_H(file, &pos->info);
						pos->info |= BIT_OO_B;
					} else {
						set_birthfile_A(file, &pos->info);
						pos->info |= BIT_OOO_B;
					}
				}
				break;
			default:
				fatal("sfen: Invalid castling file `%c'", token[i]);
			}
		}
	}

	/* En passant square (only acknowledge actual en passant opportunities) */
	token = strtok(NULL, " ");
	if (strlen(token) > 2) {
		fatal("sfen: En passant square field `%s' exceeds 2 characters", token);
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
			fatal("sfen: Invalid character `%c'", token[0]);
		}
		switch (token[1]) {
		case '3': rank = RANK_3; break;
		case '6': rank = RANK_6; break;
		default:
			fatal("sfen: Invalid character `%c'", token[0]);
		}
		/* Check if we can in fact make an EP capture */
		ep_pawn_rank = (rank == RANK_3) ? RANK_4 : RANK_5;
		sq = sq_from(file, ep_pawn_rank);
		ep_pawn_adjacent = MOVES_K[sq] & BIT_RANK[ep_pawn_rank];
		if (ep_pawn_adjacent & pos->piece[pos->info & BIT_TURN][P])
			set_ep_sq(sq_from(file, rank), &pos->info);
	}

	/* Fifty move clock (0 to 100 plies) */
	token = strtok(NULL, " ");
	fmr = atoi(token);
	set_FMR(fmr, &pos->info);

	/*
	 * Whole move clock
	 *
	 * Since this info is not essential, we do not store it in the pos->info
	 * bitmap, but instead in a different global variable.
	 */
	token = strtok(NULL, " ");
	FULL_MOVE_NUMBER = atoi(token);

	/* release resources */
	free(tokenize_me);
}

char *export_sfen(struct position *pos, char *str, int fmn)
{
	int f, r, sq, empties, color, num, idx, i;
	char cas_str[] = "ABCDEFGHabcdefgh";
	char plies_str[6] = {'\0'};
	empties = 0;
	idx = 0;

	/* Piece placement */
	for (r = RANK_8; r >= RANK_1; r--) {
		for (f = FILE_A; f != FILE_NONE; f++) {
			sq = sq_from(f, r);
			if (pos->piece_on[sq] != PIECE_NONE) {
				/* record preceding empty squares */
				if (empties) {
					str[idx] = '0' + empties;
					idx++;
					empties = 0;
				}
				switch (pos->piece_on[sq]) {
				case K: str[idx] = (pos->piece[W][K] & BIT[sq]) ? 'K' : 'k'; break;
				case Q: str[idx] = (pos->piece[W][Q] & BIT[sq]) ? 'Q' : 'q'; break;
				case R: str[idx] = (pos->piece[W][R] & BIT[sq]) ? 'R' : 'r'; break;
				case X: str[idx] = (pos->piece[W][X] & BIT[sq]) ? 'B' : 'b'; break;
				case N: str[idx] = (pos->piece[W][N] & BIT[sq]) ? 'N' : 'n'; break;
				case P: str[idx] = (pos->piece[W][P] & BIT[sq]) ? 'P' : 'p'; break;
				}
				idx++;
			} else
				empties++;

			if (f == FILE_H && empties) {
				str[idx] = '0' + empties;
				idx++;
			}
		}
		empties = 0;
		if (r > RANK_1) {
			str[idx] = '/';
			idx++;
		}
	}
	str[idx] = ' ';
	idx++;

	/* Side to move (0 is White, 1 is Black) */
	str[idx] = (pos->info & BIT_TURN) ? 'b' : 'w';
	idx++;
	str[idx] = ' ';
	idx++;

	/* Castling rights */
	if (cas_rights(&pos->info)) {
		for (color = W; color != COLOR_NONE; color++) {
			if (can_OO(color, &pos->info)) {
				str[idx] = cas_str[birthfile_H(&pos->info) + (color * 8)];
				idx++;
			}
			if (can_OOO(color, &pos->info)) {
				str[idx] = cas_str[birthfile_A(&pos->info) + (color * 8)];
				idx++;
			}
		}
	} else {
		str[idx] = '-';
		idx++;
	}
	str[idx] = ' ';
	idx++;

	/* En passant square */
	if (ep_sq(&pos->info) != SQ_NONE) {
		str[idx] = file_to_char(file_from(ep_sq(&pos->info)));
		idx++;
		str[idx] = rank_to_char(rank_from(ep_sq(&pos->info)));
		idx++;
	} else {
		str[idx] = '-';
		idx++;
	}
	str[idx] = ' ';
	idx++;

	/* Fifty Move Rule */
	num = plies_FMR(&pos->info);
	if (num) {
		i = 0;
		while (num) {
			plies_str[i++] = '0' + (num % 10);
			num /= 10;
		}
		if (i) {
			while (i)
				str[idx++] = plies_str[--i];
		} else {
			str[idx] = plies_str[0];
			idx++;
		}
	} else {
		str[idx] = '0';
		idx++;
	}
	str[idx] = ' ';
	idx++;

	/* Full move number */

	num = fmn;
	if (num) {
		for (i = 0; i < 6; i++) {
			plies_str[i] = '\0';
		}
		i = 0;
		while (num) {
			plies_str[i++] = '0' + (num % 10);
			num /= 10;
		}
		if (i) {
			while (i)
				str[idx++] = plies_str[--i];
		} else {
			str[idx] = plies_str[0];
			idx++;
		}
	} else {
		str[idx] = '0';
		idx++;
	}
	str[idx] = '\0';

	return str;
}

void pos_clear(struct position *pos)
{
	int i, j;

	for (i = 0; i != COLOR_NONE; i++) {
		for (j = 0; j != PIECE_NONE; j++) {
			pos->piece[i][j] = 0x0ULL;
		}
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
	printf("     a   b   c   d   e   f   g   h\n");
	printf("   \e[0;34m+---+---+---+---+---+---+---+---+\e[0m\n");
	for (rank = RANK_8; rank >= RANK_1; rank--) {
		set_pieces_rank(rank, pos, pieces);
		printf(" %d ", rank + 1);
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
		printf("   \e[0;34m+---+---+---+---+---+---+---+---+\e[0m\n");
	}
	printf("     a   b   c   d   e   f   g   h\n");

	/* Side to move */
	printf("\nSide to move: %s\n", (pos->info & BIT_TURN) ? "Black" : "White");
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
	if (!(pos->info & BIT_OO_W) && !(pos->info & BIT_OOO_W))
		printf("None \n");
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
	if (!(pos->info & BIT_OO_B) && !(pos->info & BIT_OOO_B))
		printf("None \n");
	/* En passant (true EP shown only, not "EP" square from FEN) */
	if (pos->info & BITS_EP_SQ) {
		sq = (pos->info & BITS_EP_SQ) >> SHF_EP_SQ;
		printf("En Passant square: ");
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
	int color, piece, sq, idx;

	idx = 0;

	for (sq = rank * 8; sq < ((rank * 8) + 8); sq++, idx++) {
		pieces[idx] = ' ';
		for (color = W; color != COLOR_NONE; color++) {
			for (piece = K; piece != PIECE_NONE; piece++) {
				if (BIT[sq] & pos->piece[color][piece]) {
					switch (piece) {
					case K: pieces[idx] = 'K'; break;
					case Q: pieces[idx] = 'Q'; break;
					case R: pieces[idx] = 'R'; break;
					case X: pieces[idx] = 'B'; break;
					case N: pieces[idx] = 'N'; break;
					case P: pieces[idx] = 'P'; break;
					default: assert(0);
					}
					/* if black, lowercase it */
					if (color == B)
						pieces[idx] += 0x20;
					break;
				}
			}
		}
	}
	pieces[8] = '\0';
}

char *disp_color(u64 *pos_info)
{
	return (our_color(pos_info) ? "Black" : "White");
}
