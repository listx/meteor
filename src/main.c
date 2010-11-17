#include "test.h"

enum {
	MODE_PERFT = 256, /* outside of the ASCII character map */
	MODE_NONE
};

void disp_help();
void disp_ver();

int main(int argc, char **argv)
{
	int o, mode;
	int plydepth;
	char *sfen;

	mode = MODE_NONE;
	plydepth = 0;
        sfen = NULL;

	/* Disable I/O buffering */
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);

        /* Program options */
	struct option longopts[] = {
		{"help",	0,	NULL,	'h'},
		{"version",	0,	NULL,	'v'},
		{"sfen",	1,	NULL,	's'},
		{"perft",	1,	NULL,	MODE_PERFT},
		{0,0,0,0}
	};

	if (argc == 1)
		disp_help();

	while ((o = getopt_long(argc, argv, "hvp:s:", longopts, NULL)) != -1) {
		switch (o) {
		case 'h':
			disp_help();
			break;
		case 'v':
			disp_ver();
			break;
		case 's':
			sfen = optarg;
			break;
		case MODE_PERFT:
			mode = MODE_PERFT;
			plydepth = atoi(optarg);
			break;
		default:
			error("unclean arguments\n");
			break;
		}
	}

	init_bitmasks();
	init_bitmasks_moves();
	init_pawn_attacks();
	init_attacks();

	switch (mode) {
	case MODE_PERFT:
		if (sfen && plydepth)
			test_perft(sfen, plydepth);
		else
			error("perft: need sfen and plydepth");

		break;
	default:
		printf("Nothing to do.\n");
		break;
	}

        return 0;
}

void disp_help()
{
	printf("Usage: meteor [OPTIONS]\n");
	printf("  -h, --help        Show help message\n");
	printf("  -v, --version     Show version\n");
	printf("  -d, --depth       Set plydepth\n");
	printf("  -s, --sfen        Set Shredder-FEN\n");
	printf("      --perft       Do perft\n");
	exit(0);
}

void disp_ver()
{
	printf("Meteor 0.02\n");
	exit(0);
}
