/*
 *	@(#)stat.c	1.3 98/11/30 Connectathon Testsuite
 *	1.4 Lachman ONC Test Suite source
 *
 * stat all of the files in a directory tree
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include <unistd.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef SVR3
#include	<sys/fs/nfs/time.h>
#else
#include	<sys/time.h>
#endif
#ifdef use_directs
#include	<sys/dir.h>
#else
#include	<dirent.h>
#endif
#endif /* DOSorWIN32 */

#include "../tests.h"

static int stats = 0;

static void statit ARGS_((char *));

main(argc, argv)
	int argc;
	char *argv[];
{
	struct timeval etim;
	float elapsed;

	if (argc != 2) {
		fprintf(stderr, "usage: %s dir\n", argv[0]);
		exit(1);
	}

	starttime();
	statit(argv[1]);
	endtime(&etim);
	elapsed = (float)etim.tv_sec + (float)etim.tv_usec / 1000000.0;
	fprintf(stdout, "%d calls in %f seconds (%f calls/sec)\n",
	    stats, elapsed, (float)stats / elapsed);
	exit(0);
}

static void
statit(name)
	char *name;
{
	struct stat statb;
#ifdef use_directs
	struct direct *di;
#else
	struct dirent *di;
#endif
	DIR *dirp;
	long loc;

#ifdef SVR3
	if (stat(name, &statb) < 0) {
#else
	if (lstat(name, &statb) < 0) {
#endif
		fprintf(stderr, "can't stat %s: %s\n",
			name, strerror(errno));
	}
	if ((statb.st_mode & S_IFMT) != S_IFDIR) {
		return;
	}

	if ((dirp = opendir(name)) == NULL) {
		fprintf(stderr, "can't opendir %s: %s\n",
			name, strerror(errno));
		return;
	}
	stats++;
	if (chdir(name) < 0) {
		fprintf(stderr, "can't chdir to %s: %s\n",
			name, strerror(errno));
		exit(1);
	}

	while ((di = readdir(dirp)) != NULL) {
		if (strcmp(di->d_name, ".") == 0 || strcmp(di->d_name, "..") == 0)
		    continue;
#ifdef SVR3
		if (stat(di->d_name, &statb) < 0)
#else
		if (lstat(di->d_name, &statb) < 0)
#endif
		{
			fprintf(stderr, "can't stat %s: %s\n",
				di->d_name, strerror(errno));
		}
		stats++;
		if ((statb.st_mode & S_IFMT) == S_IFDIR) {
			loc = telldir(dirp);
			closedir(dirp);
			statit(di->d_name);
			if ((dirp = opendir(".")) == NULL) {
				fprintf(stderr, "can't re-opendir: %s\n",
					strerror(errno));
				if (chdir("..") < 0) {
					perror("..");
					exit(1);
				}
				return;
			}
			seekdir(dirp, loc);
		}
	}

	closedir(dirp);
	if (chdir("..") < 0) {
		perror("..");
		exit(1);
	}
}
