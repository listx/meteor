#include "test.h"

#define METEOR_VERSION "0.1.1"

enum {
	MODE_PERFT = 256, /* outside of the ASCII character map */
	MODE_PERFT_ZOB,
	MODE_KISST,
	MODE_NONE
};

void disp_help(void);
void disp_ver(void);
void initialize(void);
void cap(int *val, int limit);

int main(int argc, char **argv)
{
	int o, mode;
	int plydepth, threads;
	char *sfen;

	mode = MODE_NONE;
	plydepth = 2;
	threads = sysconf(_SC_NPROCESSORS_ONLN);
        sfen = NULL;
	hashMB = 0;
	test_zob = 0;

	/* Disable I/O buffering */
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);

        /* Program options */
	struct option longopts[] = {
		{"help",	0,	NULL,	'h'},
		{"version",	0,	NULL,	'v'},
		{"hash",	1,	NULL,	'H'},
		{"sfen",	1,	NULL,	's'},
		{"threads",	1,	NULL,	't'},
		{"perft",	1,	NULL,	MODE_PERFT},
		{"perft-zob",	1,	NULL,	MODE_PERFT_ZOB},
		{"kisst",	0,	NULL,	MODE_KISST},
		{0,0,0,0}
	};

	if (argc == 1)
		disp_help();

	while ((o = getopt_long(argc, argv, "hvH:s:t:", longopts, NULL)) != -1) {
		switch (o) {
		case 'h':
			disp_help();
			break;
		case 'v':
			disp_ver();
			break;
		case 'H':
			if (atoi(optarg) >= hashMB)
				hashMB = atoi(optarg);
			cap(&hashMB, 1024); /* maximum allowed is 1024 */
			break;
		case 's':
			sfen = optarg;
			break;
		case 't':
			if (atoi(optarg) <= threads)
				threads = atoi(optarg);
			cap(&threads, 256); /* maximum threads is 256 */
			break;
		case MODE_PERFT:
			mode = MODE_PERFT;
			if (atoi(optarg) > plydepth)
				plydepth = atoi(optarg);
			break;
		case MODE_PERFT_ZOB:
			mode = MODE_PERFT_ZOB;
			if (atoi(optarg) > plydepth)
				plydepth = atoi(optarg);
			break;
		case MODE_KISST:
			mode = MODE_KISST;
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
		init_zob();
		if (sfen && plydepth)
			test_perft(sfen, plydepth, threads);
		else
			error("perft: need sfen and plydepth");
		break;
	case MODE_PERFT_ZOB:
		init_zob();
		test_zob = 1;
		threads = 1;
		if (sfen && plydepth)
			test_perft(sfen, plydepth, threads);
		else
			error("perft: need sfen and plydepth");
		break;
	case MODE_KISST:
		randk_seed(); /* initialize PRNG with default seed */
		randk_verify();
		break;
	default:
		printf("Nothing to do.\n");
		break;
	}

        return 0;
}

void disp_help(void)
{
	printf("Usage: meteor [OPTIONS]\n");
	printf("  -h --help           Show help message\n");
	printf("  -v --version        Show version\n");
	printf("  -H --hash NUM       Set hashtable size in MiB (default 0; max 1024)\n");
	printf("  -s --sfen STRING    Set Shredder-FEN (use this with --perft NUM)\n");
	printf("  -t --threads NUM    Use NUM threads\n");
	printf("     --perft NUM      Do perft up to ply depth NUM (ignored if NUM < 2)\n");
	printf("     --perft-zob NUM  Same as --perft, but also check if zobrist keys\n");
	printf("                        are being properly used. The hashtable is disabled\n");
	printf("                        for this mode. Also, this mode forces --threads to\n");
	printf("                        be 1\n");
	printf("     --kisst          Verify if meteor's PRNG is working correctly.\n");
	exit(0);
}

void disp_ver(void)
{
	printf("meteor version %s\n", METEOR_VERSION);
	exit(0);
}

void initialize(void)
{
	init_bitmasks();
	init_bitmasks_moves();
	init_pawn_attacks();
	init_attacks();
}

void cap(int *val, int limit)
{
	if (*val > limit)
		*val = limit;
}
