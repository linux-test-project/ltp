#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <err.h>

#include "../cpuset_lib/common.h"
#include "../cpuset_lib/bitmask.h"

#define MAX_STRING_SIZE	400
#define MAX_NBITS	1024

#define USAGE  ("Usage : %s [-a|s] list1 [list2]\n"			\
		"\t-a|s   list1 add/subtract list2."			\
		"[default: -a]\n"					\
		"\t-h     Help.\n")
int add_or_subtract;
int convert;

static void usage(char *prog_name, int status)
{
	fprintf(stderr, USAGE, prog_name);
	exit(status);
}

static void checkopt(int argc, char **argv)
{
	int c, optc = 0;

	while ((c = getopt(argc, argv, "ahs")) != -1) {
		switch (c) {
		case 'a':
			add_or_subtract = 0;
			optc++;
			break;
		case 'h':	/* help */
			usage(argv[0], 0);
			break;
		case 's':
			add_or_subtract = 1;
			optc++;
			break;
		default:
			usage(argv[0], 1);
			break;
		}
	}

	if (optc == 2)
		OPT_COLLIDING(argv[0], 'a', 's');

	if (argc == optc + 1) {
		fprintf(stderr, "%s: missing the argument list1.\n", argv[0]);
		usage(argv[0], 1);
	} else if (argc == optc + 2)
		convert = 1;
}

int main(int argc, char **argv)
{
	struct bitmask *mask1 = NULL, *mask2 = NULL, *mask3 = NULL;
	char buff[MAX_STRING_SIZE];

	checkopt(argc, argv);

	mask1 = bitmask_alloc(MAX_NBITS);
	if (mask1 == NULL)
		err(EXIT_FAILURE, "alloc memory space for bitmask1 failed.");

	mask2 = bitmask_alloc(MAX_NBITS);
	if (mask2 == NULL)
		err(EXIT_FAILURE, "alloc memory space for bitmask2 failed.");

	mask3 = bitmask_alloc(MAX_NBITS);
	if (mask3 == NULL)
		err(EXIT_FAILURE, "alloc memory space for bitmask3 failed.");

	if (bitmask_parselist(argv[argc - 2 + convert], mask1) != 0)
		err(EXIT_FAILURE, "parse list1 string failed.");

	if (convert) {
		bitmask_displaylist(buff, MAX_STRING_SIZE, mask1);
		printf("%s\n", buff);
		exit(0);
	}

	if (bitmask_parselist(argv[argc - 1], mask2) != 0)
		err(EXIT_FAILURE, "parse list2 string failed.");

	if (add_or_subtract)
		bitmask_andnot(mask3, mask1, mask2);
	else
		bitmask_or(mask3, mask1, mask2);

	bitmask_displaylist(buff, MAX_STRING_SIZE, mask3);
	printf("%s\n", buff);

	return 0;
}
