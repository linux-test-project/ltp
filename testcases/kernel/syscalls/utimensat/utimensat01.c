/*************************************************************************************/
/*                                                                                   */
/* Copyright (C) 2008, Michael Kerrisk <mtk.manpages@gmail.com>,                     */
/* Copyright (C) 2008, Linux Foundation                                              */
/*                                                                                   */
/* This program is free software;  you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by              */
/* the Free Software Foundation; either version 2 of the License, or                 */
/* (at your option) any later version.                                               */
/*                                                                                   */
/* This program is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of                   */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                         */
/* the GNU General Public License for more details.                                  */
/*                                                                                   */
/* You should have received a copy of the GNU General Public License                 */
/* along with this program;  if not, write to the Free Software                      */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           */
/*************************************************************************************/
/*                                                                                   */
/* File: utimnsat01.c                                                                */
/* Description: A command-line interface for testing the utimensat() system call.    */
/* Author: Michael Kerrisk <mtk.manpages@gmail.com>                                  */
/* History:                                                                          */
/*	17 Mar  2008  Initial creation,                                              */
/*	31 May  2008  Reworked for easier test automation,                           */
/*	2  June 2008  Renamed from t_utimensat.c to test_utimensat.c,                */
/*	05 June 2008  Submitted to LTP by Subrata Modak <subrata@linux.vnet.ibm.com> */
/*************************************************************************************/

#define _GNU_SOURCE
#define _ATFILE_SOURCE
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "utimensat01";
int TST_TOTAL = 0;

#define cleanup tst_exit

/* We use EXIT_FAILURE for an expected failure from utimensat()
   (e.g., EACCES and EPERM), and one of the following for unexpected
   failures (i.e., something broke in our test setup). */

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100
#endif

#define EXIT_bad_usage 3
#define EXIT_failed_syscall 3

#define errExit(msg)    do { perror(msg); exit(EXIT_failed_syscall); \
                        } while (0)

#define UTIME_NOW      ((1l << 30) - 1l)
#define UTIME_OMIT     ((1l << 30) - 2l)

static inline int
utimensat_sc(int dirfd, const char *pathname,
	     const struct timespec times[2], int flags)
{
	return ltp_syscall(__NR_utimensat, dirfd, pathname, times, flags);
}

static void usageError(char *progName)
{
	fprintf(stderr, "Usage: %s pathname [atime-sec "
		"atime-nsec mtime-sec mtime-nsec]\n\n", progName);
	fprintf(stderr, "Permitted options are:\n");
	fprintf(stderr, "    [-d path] "
		"open a directory file descriptor"
		" (instead of using AT_FDCWD)\n");
	fprintf(stderr, "    -q        Quiet\n");
	fprintf(stderr, "    -w        Open directory file "
		"descriptor with O_RDWR|O_APPEND\n"
		"              (instead of O_RDONLY)\n");
	fprintf(stderr, "    -n        Use AT_SYMLINK_NOFOLLOW\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "pathname can be \"NULL\" to use NULL "
		"argument in call\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "Either nsec field can be\n");
	fprintf(stderr, "    'n' for UTIME_NOW\n");
	fprintf(stderr, "    'o' for UTIME_OMIT\n");
	fprintf(stderr, "\n");

	fprintf(stderr, "If the time fields are omitted, "
		"then a NULL 'times' argument is used\n");
	fprintf(stderr, "\n");

	exit(EXIT_bad_usage);
}

int main(int argc, char *argv[])
{
	int flags, dirfd, opt, oflag;
	struct timespec ts[2];
	struct timespec *tsp;
	char *pathname, *dirfdPath;
	struct stat sb;
	int verbose;

	/* Command-line argument parsing */

	flags = 0;
	verbose = 1;
	dirfd = AT_FDCWD;
	dirfdPath = NULL;
	oflag = O_RDONLY;

	while ((opt = getopt(argc, argv, "d:nqw")) != -1) {
		switch (opt) {
		case 'd':
			dirfdPath = optarg;
			break;

		case 'n':
			flags |= AT_SYMLINK_NOFOLLOW;
			if (verbose)
				printf("Not following symbolic links\n");
			break;

		case 'q':
			verbose = 0;
			break;

		case 'w':
			oflag = O_RDWR | O_APPEND;
			break;

		default:
			usageError(argv[0]);
		}
	}

	if ((optind + 5 != argc) && (optind + 1 != argc))
		usageError(argv[0]);

	if (dirfdPath != NULL) {
		dirfd = open(dirfdPath, oflag);
		if (dirfd == -1)
			errExit("open");

		if (verbose) {
			printf("Opened dirfd %d", oflag);
			if ((oflag & O_ACCMODE) == O_RDWR)
				printf(" O_RDWR");
			if (oflag & O_APPEND)
				printf(" O_APPEND");
			printf(": %s\n", dirfdPath);
		}
	}

	pathname = (strcmp(argv[optind], "NULL") == 0) ? NULL : argv[optind];

	/* Either, we get no values for 'times' fields, in which case
	   we give a NULL pointer to utimensat(), or we get four values,
	   for secs+nsecs for each of atime and mtime.  The special
	   values 'n' and 'o' can be used for tv_nsec settings of
	   UTIME_NOW and UTIME_OMIT, respectively. */

	if (argc == optind + 1) {
		tsp = NULL;

	} else {
		ts[0].tv_sec = atoi(argv[optind + 1]);
		if (argv[optind + 2][0] == 'n') {
			ts[0].tv_nsec = UTIME_NOW;
		} else if (argv[optind + 2][0] == 'o') {
			ts[0].tv_nsec = UTIME_OMIT;
		} else {
			ts[0].tv_nsec = atoi(argv[optind + 2]);
		}

		ts[1].tv_sec = atoi(argv[optind + 3]);
		if (argv[optind + 4][0] == 'n') {
			ts[1].tv_nsec = UTIME_NOW;
		} else if (argv[optind + 4][0] == 'o') {
			ts[1].tv_nsec = UTIME_OMIT;
		} else {
			ts[1].tv_nsec = atoi(argv[optind + 4]);
		}

		tsp = ts;
	}

	/* For testing purposes, it may have been useful to run this program
	   as set-user-ID-root so that a directory file descriptor could be
	   opened as root.  (This allows us to obtain a file descriptor even
	   if normal user doesn't have permissions on the file.)  Now we
	   reset to the real UID before making the utimensat() call, so that
	   the permission checking for the utimensat() call is performed
	   under that UID. */

	if (geteuid() == 0) {
		uid_t u;

		u = getuid();

		if (verbose)
			printf("Resetting UIDs to %ld\n", (long)u);

		if (setresuid(u, u, u) == -1)
			errExit("setresuid");
	}

	/* Display information allowing user to verify arguments for call */

	if (verbose) {
		printf("dirfd is %d\n", dirfd);
		printf("pathname is %s\n", pathname);
		printf("tsp is %p", tsp);
		if (tsp != NULL) {
			printf("; struct  = { %ld, %ld } { %ld, %ld }",
			       (long)tsp[0].tv_sec, (long)tsp[0].tv_nsec,
			       (long)tsp[1].tv_sec, (long)tsp[1].tv_nsec);
		}
		printf("\n");
		printf("flags is %d\n", flags);
	}

	/* Make the call and see what happened */

	if (utimensat_sc(dirfd, pathname, tsp, flags) == -1) {
		if (errno == EPERM) {
			if (verbose)
				printf("utimensat() failed with EPERM\n");
			else
				printf("EPERM\n");
			exit(EXIT_FAILURE);

		} else if (errno == EACCES) {
			if (verbose)
				printf("utimensat() failed with EACCES\n");
			else
				printf("EACCES\n");
			exit(EXIT_FAILURE);

		} else if (errno == EINVAL) {
			if (verbose)
				printf("utimensat() failed with EINVAL\n");
			else
				printf("EINVAL\n");
			exit(EXIT_FAILURE);

		} else {	/* Unexpected failure case from utimensat() */
			errExit("utimensat");
		}
	}

	if (verbose)
		printf("utimensat() succeeded\n");

	if (stat((pathname != NULL) ? pathname : dirfdPath, &sb) == -1)
		errExit("stat");

	if (verbose) {
		printf("Last file access:         %s", ctime(&sb.st_atime));
		printf("Last file modification:   %s", ctime(&sb.st_mtime));
		printf("Last status change:       %s", ctime(&sb.st_ctime));

	} else {
		printf("SUCCESS %ld %ld\n", (long)sb.st_atime,
		       (long)sb.st_mtime);
	}

	exit(EXIT_SUCCESS);
}
