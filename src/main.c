#include "test.h"

#define METEOR_VERSION "0.04"

enum {
	MODE_PERFT = 256, /* outside of the ASCII character map */
	MODE_NONE
};

void disp_help();
void disp_ver();
void initialize();

int main(int argc, char **argv)
{
	int o, mode;
	int plydepth, threads;
	char *sfen;

	mode = MODE_NONE;
	plydepth = 2;
	threads = sysconf(_SC_NPROCESSORS_ONLN);
        sfen = NULL;

	/* Disable I/O buffering */
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);

        /* Program options */
	struct option longopts[] = {
		{"help",	0,	NULL,	'h'},
		{"version",	0,	NULL,	'v'},
		{"threads",	1,	NULL,	't'},
		{"sfen",	1,	NULL,	's'},
		{"perft",	1,	NULL,	MODE_PERFT},
		{0,0,0,0}
	};

	if (argc == 1)
		disp_help();

	while ((o = getopt_long(argc, argv, "hvs:t:", longopts, NULL)) != -1) {
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
		case 't':
			if (atoi(optarg) <= threads)
				threads = atoi(optarg);
			break;
		case MODE_PERFT:
			mode = MODE_PERFT;
			if (atoi(optarg) > plydepth)
				plydepth = atoi(optarg);
			break;
		default:
			fatal("unclean arguments\n");
			break;
		}
	}

	/* exit if there were any unrecognized arguments */
	if (optind < argc)
		fatal("unrecognize option: `%s'", argv[optind]);

	if (mode != MODE_NONE)
		initialize();

	switch (mode) {
	case MODE_PERFT:
		if (sfen && plydepth)
			test_perft(sfen, plydepth, threads);
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
	printf("  -h --help           Show help message\n");
	printf("  -v --version        Show version\n");
	printf("  -s --sfen STRING    Set Shredder-FEN (use this with --perft NUM)\n");
	printf("  -t --threads NUM    Use NUM threads\n");
	printf("     --perft NUM      Do perft up to ply depth NUM (ignored if NUM < 2)\n");
	exit(0);
}

void disp_ver()
{
	printf("meteor version %s\n", METEOR_VERSION);
	exit(0);
}

void initialize()
{
	init_bitmasks();
	init_bitmasks_moves();
	init_pawn_attacks();
	init_attacks();
}
