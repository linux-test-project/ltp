/*
 *      @(#)domount.c	1.2 97/01/03 Connectathon testsuite
 *	1.1 Lachman ONC Test Suite source
 *
 * domount [-u] [args]
 *
 * NOTE: This program should be suid root to work properly.
 */

#include <stdio.h>

main(argc, argv)
	int argc;
	char **argv;
{
	char *comm;
	extern char *getenv();

	if (argc > 1 && strcmp(argv[1], "-u") == 0) {
		if ((comm = getenv("UMOUNT")) != NULL)
			*++argv = comm;
		else
			*++argv = "/etc/umount";
	} else {
		if ((comm = getenv("MOUNT")) != NULL)
			*argv = comm;
		else
			*argv = "/etc/mount";
	}

	(void)setuid(0);

	(void)execv(*argv, argv);

	exit(1);
	/* NOTREACHED */
}
