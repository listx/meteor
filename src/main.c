#include "io.h"
#include "move.h"
#include "position.h"

void disp_help();
void disp_ver();

int main(int argc, char **argv)
{
	int o;

	/* Disable I/O buffering */
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);

        /* Program options */
	struct option longopts[] = {
		{"help",	no_argument,		0,	'h'},
		{"version",	no_argument,		0,	'v'},
		{0,0,0,0}
	};

	if (argc == 1)
		disp_help();

	while ((o = getopt_long(argc, argv, "hvt", longopts, NULL)) != -1) {
		switch (o) {
		case 'h':
			disp_help();
			break;
		case 'v':
			disp_ver();
			break;
		case 't': break;
		default:
			error("unclean arguments\n");
			break;
		}
	}

	init_bitmasks();
	init_bitmasks_moves();
	/* artificially set up the position */
	struct position position_test;
	struct position *pos;
	pos = &position_test;

	char str[] = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b HAha - 0 1";

	import_sfen(str, pos);

	/* display it */
	disp_pos(pos);

        return 0;
}

void disp_help()
{
	printf("Usage: meteor [OPTIONS]\n");
	printf("  -h, --help        Show help message\n");
	printf("  -v, --version     Show version\n");
	exit(0);
}

void disp_ver()
{
	printf("meteor version 0.01\n");
	exit(0);
}
