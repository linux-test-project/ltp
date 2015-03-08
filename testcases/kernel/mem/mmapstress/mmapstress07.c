/* IBM Corporation */
/* 01/03/2003	Port to LTP avenkat@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 *   Copyright (c) International Business Machines  Corp., 2003
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
 *	Mmap a sparse file and then fiddle with the hole in the middle.
 *	Then check the file contents.
 *
 *  Usage: mmapstress07 filename holesize e_pageskip sparseoff
 *  EXAMPLE: mmapstress07 myfile 4096 1 4096
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include "test.h"
#define FAILED 0
#define PASSED 1

static char *tmpname;

#define ERROR(M)	(void)fprintf(stderr, "%s: errno = %d: " M "\n", \
			argv[0], errno)

#define CLEANERROR(M)	(void)close(rofd); \
			(void)close(rwfd); \
			(void)unlink(tmpname); \
			ERROR(M)

#define CATCH_SIG(SIG) \
        if (sigaction(SIG, &sa, 0) == -1) { \
                ERROR("couldn't catch signal " #SIG); \
                exit(1); \
        }

extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern void exit(int);
static int checkchars(int fd, char val, int n);

char *TCID = "mmapstress07";

int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
void ok_exit();

 /*ARGSUSED*/ static
void cleanup(int sig)
{
	/*
	 * Don't check error codes - we could be signaled before the file is
	 * created.
	 */
	(void)unlink(tmpname);
	exit(1);
}

int main(int argc, char **argv)
{
	size_t pagesize = (size_t) sysconf(_SC_PAGE_SIZE);
	caddr_t mapaddr;
	time_t t;
	int rofd, rwfd, i;
	struct sigaction sa;
	int e_pageskip;
#ifdef LARGE_FILE
	off64_t holesize;
	off64_t sparseoff;
#else /* LARGE_FILE */
	off_t holesize;
	off_t sparseoff;
#endif /* LARGE_FILE */

	(void)time(&t);
//      (void)printf("%s: Started %s", argv[0], ctime(&t));
	/* Test fsync & mmap over a hole in a sparse file & extend fragment */
	if (argc < 2 || argc > 5) {
		fprintf(stderr,
			"Usage: mmapstress07 filename holesize e_pageskip sparseoff\n");
		/*****	**	LTP Port 02/01/03	**	**** */
		fprintf(stderr,
			"\t*holesize should be a multiple of pagesize\n");
		fprintf(stderr, "\t*e_pageskip should be 1 always \n");
		fprintf(stderr,
			"\t*sparseoff should be a multiple of pagesize\n");
		fprintf(stderr, "Example: mmapstress07 myfile 4096 1 8192\n");
		/*****	**	******	*****	*****	**	02/01/03 */
		anyfail();	/* LTP Port */
	}
	tst_tmpdir();
	tmpname = argv[1];

	if (argc >= 3) {
#ifdef LARGE_FILE
		holesize = atoll(argv[2]);
#else /* LARGE_FILE */
		holesize = atoi(argv[2]);
#endif /* LARGE_FILE */
	} else
		holesize = pagesize;

	if (argc >= 4)
		e_pageskip = atoi(argv[3]);
	else
		e_pageskip = 1;

	if (argc >= 5) {
#ifdef LARGE_FILE
		sparseoff = atoll(argv[4]);
#else /* LARGE_FILE */
		sparseoff = atoi(argv[4]);
#endif /* LARGE_FILE */
	} else
		sparseoff = pagesize * 2;

	sa.sa_handler = cleanup;
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask)) {
		ERROR("sigemptyset failed");
		return 1;
	}
	CATCH_SIG(SIGINT);
	CATCH_SIG(SIGQUIT);
	CATCH_SIG(SIGTERM);
#ifdef LARGE_FILE
	if ((rofd = open64(tmpname, O_RDONLY | O_CREAT, 0777)) == -1) {
#else /* LARGE_FILE */
	if ((rofd = open(tmpname, O_RDONLY | O_CREAT, 0777)) == -1) {
#endif /* LARGE_FILE */
		ERROR("couldn't reopen rofd for reading");
		anyfail();	/* LTP Port */
	}
#ifdef LARGE_FILE
	if ((rwfd = open64(tmpname, O_RDWR)) == -1) {
#else /* LARGE_FILE */
	if ((rwfd = open(tmpname, O_RDWR)) == -1) {
#endif /* LARGE_FILE */
		CLEANERROR("couldn't reopen rwfd for read/write");
		anyfail();	/* LTP Port */
	}
#ifdef LARGE_FILE
	if (lseek64(rwfd, sparseoff, SEEK_SET) < 0) {
#else /* LARGE_FILE */
	if (lseek(rwfd, sparseoff, SEEK_SET) < 0) {
#endif /* LARGE_FILE */
		perror("lseek");
		anyfail();	/* LTP Port */
	}
	/* fill file with junk. */
	i = 0;
	while (i < pagesize && write(rwfd, "a", 1) == 1)
		i++;
	if (i != pagesize) {
		CLEANERROR("couldn't fill first part of file with junk");
		anyfail();	/* LTP Port */
	}
#ifdef LARGE_FILE
	if (lseek64(rwfd, holesize, SEEK_CUR) == -1) {
#else /* LARGE_FILE */
	if (lseek(rwfd, holesize, SEEK_CUR) == -1) {
#endif /* LARGE_FILE */
		CLEANERROR("couldn't create hole in file");
		anyfail();	/* LTP Port */
	}
	/* create fragment */
	i = 0;
	while (i < (pagesize >> 1) && write(rwfd, "b", 1) == 1)
		i++;
	if (i != (pagesize >> 1)) {
		CLEANERROR("couldn't fill second part of file with junk");
		anyfail();	/* LTP Port */
	}
	/* At this point fd contains 1 page of a's, holesize bytes skipped,
	 * 1/2 page of b's.
	 */

#ifdef LARGE_FILE
	if ((mapaddr = mmap64((caddr_t) 0, pagesize * 2 + holesize, PROT_READ,
			      MAP_SHARED | MAP_FILE, rofd,
			      sparseoff)) == (caddr_t) - 1) {
#else /* LARGE_FILE */
	if ((mapaddr = mmap((caddr_t) 0, pagesize * 2 + holesize, PROT_READ,
			    MAP_SHARED | MAP_FILE, rofd,
			    sparseoff)) == (caddr_t) - 1) {
#endif /* LARGE_FILE */
		CLEANERROR("mmap tmp file failed");
		anyfail();	/* LTP Port */
	}
	/* fill out remainder of page + one more page to extend mmapped flag */
	while (i < 2 * pagesize && write(rwfd, "c", 1) == 1)
		i++;
	if (i != 2 * pagesize) {
		CLEANERROR("couldn't fill second part of file with junk");
		anyfail();	/* LTP Port */
	}
	/* fiddle with mmapped hole */
	if (*(mapaddr + pagesize + (holesize >> 1)) != 0) {
		CLEANERROR("hole not filled with 0's");
		anyfail();	/* LTP Port */
	}
#ifdef LARGE_FILE
	if (lseek64(rwfd, sparseoff + e_pageskip * pagesize, SEEK_SET) == -1) {
#else /* LARGE_FILE */
	if (lseek(rwfd, sparseoff + e_pageskip * pagesize, SEEK_SET) == -1) {
#endif /* LARGE_FILE */
		CLEANERROR("couldn't lseek back to put e's in hole");
		anyfail();	/*LTP Port */
	}
	i = 0;
	while (i < pagesize && write(rwfd, "e", 1) == 1)
		i++;
	if (i != pagesize) {
		CLEANERROR("couldn't part of hole with e's");
		anyfail();	/*LTP Port */
	}
	if (fsync(rwfd) == -1) {
		CLEANERROR("fsync failed");
		anyfail();	/* LTP Port */
	}
#ifdef LARGE_FILE
	if (lseek64(rofd, sparseoff, SEEK_SET) == -1) {
#else /* LARGE_FILE */
	if (lseek(rofd, sparseoff, SEEK_SET) == -1) {
#endif /* LARGE_FILE */
		CLEANERROR("couldn't lseek to begining to verify contents");
		anyfail();	/* LTP Port */
	}
	if (munmap(mapaddr, holesize + 2 * pagesize) == -1) {
		CLEANERROR("munmap of tmp file failed");
		anyfail();	/* LTP Port */
	}
	/* check file's contents */
	if (checkchars(rofd, 'a', pagesize)) {
		CLEANERROR("first page not filled with a's");
		anyfail();	/* LTP Port */
	}
	if (checkchars(rofd, '\0', (e_pageskip - 1) * pagesize)) {
		CLEANERROR("e_skip not filled with 0's");
		anyfail();	/* LTP Port */
	}
	if (checkchars(rofd, 'e', pagesize)) {
		CLEANERROR("part after first 0's not filled with e's");
		anyfail();	/* LTP Port */
	}
	if (checkchars(rofd, '\0', holesize - e_pageskip * pagesize)) {
		CLEANERROR("second hole section not filled with 0's");
		anyfail();	/* LTP Port */
	}
	if (checkchars(rofd, 'b', (pagesize >> 1))) {
		CLEANERROR("next to last half page not filled with b's");
		anyfail();	/* LTP Port */
	}
	if (checkchars(rofd, 'c', pagesize + (pagesize >> 1))) {
		CLEANERROR("extended fragment not filled with c's");
		anyfail();	/* LTP Port */
	}
	if (close(rofd) == -1) {
		CLEANERROR("second close of rofd failed");
		anyfail();	/* LTP Port */
	}
	if (unlink(tmpname) == -1) {
		CLEANERROR("unlink failed");
		anyfail();	/* LTP Port */
	}
	(void)time(&t);
//      (void)printf("%s: Finished %s", argv[0], ctime(&t));
	ok_exit();
	tst_exit();
}

/* checkchars
 * 	verrify that the next n characters of file fd are of value val.
 *	0 = success; -1 = failure
 */
static int checkchars(int fd, char val, int n)
{
	int i;
	char buf;

	for (i = 0; i < n && read(fd, &buf, 1) == 1; i++)
		if (buf != val)
			return -1;
	return 0;
}

/*****	**	LTP Port	**	*****/
int anyfail(void)
{
	tst_brkm(TFAIL, tst_rmdir, "Test failed\n");
}

void ok_exit(void)
{
	tst_resm(TPASS, "Test passed\n");
	tst_rmdir();
	tst_exit();
}

/*****	**	******		**	*****/
