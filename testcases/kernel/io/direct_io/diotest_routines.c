/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * NAME
 *      diotest_routines.c
 *
 * DESCRIPTION
 *	Functions that are used in diotest programs.
 *	fillbuf(), bufcmp(), filecmp()
 *	forkchldrn(), waitchldrn(), killchldrn()
 *
 * History
 *	04/10/2002	Narasimha Sharoff
 *
 * RESTRICTIONS
 *	None
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include "diotest_routines.h"

/* **** Routines for buffer actions, comparisions **** */

/*
 * fillbuf: Fill buffer of given size with given character value
 * vfillbuf: Fill the vector array
*/
void fillbuf(char *buf, int count, char value)
{
	while (count > 0) {
		strncpy(buf, &value, 1);
		buf++;
		count = count - 1;
	}
}

void vfillbuf(struct iovec *iv, int vcnt, char value)
{
	int i;

	for (i = 0; i < vcnt; iv++, i++) {
		fillbuf(iv->iov_base, iv->iov_len, (char)value);
	}
}

/*
 * bufcmp: Compare two buffers
 * vbufcmp: Compare two buffers of two io arrays
*/
int bufcmp(char *b1, char *b2, int bsize)
{
	int i;

	for (i = 0; i < bsize; i++) {
		if (strncmp(&b1[i], &b2[i], 1)) {
			fprintf(stderr,
				"bufcmp: offset %d: Expected: 0x%x, got 0x%x\n",
				i, b1[i], b2[i]);
			return (-1);
		}
	}
	return (0);
}

int vbufcmp(struct iovec *iv1, struct iovec *iv2, int vcnt)
{
	int i;

	for (i = 0; i < vcnt; iv1++, iv2++, i++) {
		if (bufcmp(iv1->iov_base, iv2->iov_base, iv1->iov_len) < 0) {
			fprintf(stderr, "Vector: %d, iv1base=%s, iv2base=%s\n",
				i, (char *)iv1->iov_base,
				(char *)iv2->iov_base);
			return (-1);
		}
	}
	return 0;
}

/*
 * compare_files: Compares two files
*/
int filecmp(char *f1, char *f2)
{
	int i;
	int fd1, fd2;
	int ret1, ret2 = 0;
	char buf1[BUFSIZ], buf2[BUFSIZ];

	/* Open the file for read */
	if ((fd1 = open(f1, O_RDONLY)) == -1) {
		fprintf(stderr, "compare_files: open failed %s: %s",
			f1, strerror(errno));
		return (-1);
	}
	if ((fd2 = open(f2, O_RDONLY)) == -1) {
		fprintf(stderr, "compare_files: open failed %s: %s",
			f2, strerror(errno));
		close(fd1);
		return (-1);
	}

	/* Compare the files */
	while ((ret1 = read(fd1, buf1, BUFSIZ)) > 0) {
		ret2 = read(fd2, buf2, BUFSIZ);
		if (ret1 != ret2) {
			fprintf(stderr, "compare_file: file length mistmatch:");
			fprintf(stderr, "read: %d from %s, %d from %s",
				ret1, f1, ret2, f2);
			close(fd1);
			close(fd2);
			return (-1);
		}
		for (i = 0; i < ret1; i++) {
			if (strncmp(&buf1[i], &buf2[i], 1)) {
				fprintf(stderr, "compare_file: char mismatch:");
				fprintf(stderr, " %s offset %d: 0x%02x %c  ",
					f1, i, buf1[i],
					isprint(buf1[i]) ? buf1[1] : '.');
				fprintf(stderr, " %s offset %d: 0x%02x %c\n",
					f2, i, buf2[i],
					isprint(buf2[i]) ? buf2[i] : '.');
				close(fd1);
				close(fd2);
				return (-1);
			}
		}
	}
	close(fd1);
	close(fd2);
	return 0;
}

/* **** Routines to create, wait and destroy child processes **** */

/*
 * forkchldrn: fork the given number of children and set the function
 *		that child should execute.
*/
int forkchldrn(int **pidlst, int numchld, int action, int (*chldfunc) ())
{
	int i, cpid;

	if ((*pidlst = ((int *)malloc(sizeof(int) * numchld))) == 0) {
		fprintf(stderr, "forkchldrn: calloc failed for pidlst: %s\n",
			strerror(errno));
		return (-1);
	}
	for (i = 0; i < numchld; i++) {
		if ((cpid = fork()) < 0) {
			fprintf(stderr,
				"forkchldrn: fork child %d failed, %s\n", i,
				strerror(errno));
			killchldrn(pidlst, i, SIGTERM);
			return (-1);
		}
		if (cpid == 0)
			exit((*chldfunc) (i, action));
		else
			*(*pidlst + i) = cpid;
	}
	return 0;
}

/*
 * killchldrn: signal the children listed in pidlst with the given signal
 *
*/
int killchldrn(int **pidlst, int numchld, int sig)
{
	int i, cpid, errflag = 0;

	for (i = 0; i < numchld; i++) {
		cpid = *(*pidlst + i);
		if (cpid > 0) {
			if (kill(cpid, sig) < 0) {
				fprintf(stderr,
					"killchldrn: kill %d failed, %s\n",
					cpid, strerror(errno));
				errflag--;
			}
		}
	}
	return (errflag);
}

/*
 * waitchldrn: wait for child process listed in pidlst to finish.
*/
int waitchldrn(int **pidlst, int numchld)
{
	int i, cpid, ret, errflag = 0;
	int status;

	for (i = 0; i < numchld; i++) {
		cpid = *(*pidlst + i);
		if (cpid == 0)
			continue;
		if ((ret = waitpid(cpid, &status, 0)) != cpid) {
			fprintf(stderr,
				"waitchldrn: wait failed for child %d, pid %d: %s\n",
				i, cpid, strerror(errno));
			errflag--;
		}
		if (status)
			errflag = -1;
	}
	return (errflag);
}
