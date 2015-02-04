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

/* 11/18/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	data_space.c -- test data space
 *
 * CALLS
 *	malloc (3)
 *
 * ALGORITHM
 *	Test VM for set of data-space intensive programs
 *
 */

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//void (*sigset(int, void(*)(int)))(int);

/** LTP Port **/
#include "test.h"

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
int block_number;

char *TCID = "data_space";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
/**************/

#define MAXCHILD	100	/* max number of children to allow */
int allchild[MAXCHILD + 1];
#define K_1		1024
#define K_2		2048
#define K_4		4096

#define bd_arg(str) \
	tst_brkm(TCONF, NULL, \
	    "bad argument - %s - could not parse as number.", str)

int nchild;			/* # kids */
int csize;			/* chunk size */
int iterations;			/* # total iterations */
int rep_freq;			/* report frequency */
int max_size;			/* max file size */
int parent_pid;

int usage(char *);
int runtest();
int dotest(int, int);
void bfill(char *, char, int);
int dumpbuf(char *);
void dumpbits(char *, int);
int massmurder();
int okexit(int);

char *prog;			/* invoked name */
int chld_flag = 0;

void cleanup(void)
{
	tst_rmdir();
}

int usage(prog)
char *prog;
{
	tst_resm(TCONF, "Usage: %s <nchild> <size> <chunk_size> <iterations>",
		 prog);
	tst_brkm(TCONF, NULL, "DEFAULTS: 10 1024*1024 4096 25");
}

int main(argc, argv)
int argc;
char *argv[];
{
	int i = 1;
	int term();
	int chld();

	prog = argv[0];

	if (argc == 1) {
		nchild = 10;
		max_size = K_1 * K_1;
		csize = K_4;
		iterations = 25;
	} else if (argc == 5) {
		if (sscanf(argv[i++], "%d", &nchild) != 1)
			bd_arg(argv[i - 1]);
		if (sscanf(argv[i++], "%d", &max_size) != 1)
			bd_arg(argv[i - 1]);
		if (sscanf(argv[i++], "%d", &csize) != 1)
			bd_arg(argv[i - 1]);
		if (sscanf(argv[i++], "%d", &iterations) != 1)
			bd_arg(argv[i - 1]);
		if (nchild > MAXCHILD) {
			tst_brkm(TBROK, NULL,
				 "FAILURE, %d children exceeded maximum allowed",
				 nchild);
		}
	} else
		usage(prog);

	tst_tmpdir();

	parent_pid = getpid();

	if (sigset(SIGTERM, (void (*)())term) == SIG_ERR) {
		tst_brkm(TBROK, NULL, "first sigset failed");
	}
	if (sigset(SIGUSR1, (void (*)())chld) == SIG_ERR) {
		tst_brkm(TBROK, NULL, "sigset shichld");
	}

	runtest();
	tst_exit();
}

int runtest()
{
	register int i;
	int child;
	int status;
	int count;

	for (i = 0; i < nchild; i++) {
		chld_flag = 0;
		switch (child = fork()) {
		case -1:
			tst_brkm(TBROK | TERRNO, cleanup, "fork failed");
		case 0:
			dotest(nchild, i);
			exit(0);
		}
		allchild[i] = child;
		while (!chld_flag)
			sleep(1);
	}

	/*
	 * Wait for children to finish.
	 */

	count = 0;
	while ((child = wait(&status)) > 0) {
#ifdef DEBUG
		tst_resm(TINFO, "\t%s[%d] exited status = 0x%x\n", prog, child,
			 status);
#endif
		if (status) {
			tst_resm(TFAIL, "\tTest failed, expected 0 exit.\n");
			local_flag = FAILED;
		}
		++count;
	}

	/*
	 * Should have collected all children.
	 */

	if (count != nchild) {
		tst_resm(TFAIL, "\tWrong # children waited on, count = %d\n",
			 count);
		local_flag = FAILED;
	}

	if (local_flag == FAILED)
		tst_resm(TFAIL, "Test failed");
	else
		tst_resm(TPASS, "Test passed");
	sync();			/* safeness */

	return 0;
}

/*
 * dotest()
 *	Children execute this.
 *
 * Randomly read/mod/write chunks with known pattern and check.
 * When fill sectors, iterate.
 *
 */

int nchunks;

#define	CHUNK(i)	((i) * csize)

int dotest(testers, me)
int testers;
int me;
{
	char *bits;
	char *mondobuf;
	char *val_buf;
	char *zero_buf;
	char *buf;
	int count;
	int collide;
	char val;
	int chunk;

	/*
	 * Do the mondo-test.
	 *
	 * NOTE: If we run this with a lot of children, the last child
	 *       processes may not have enough swap space to do these
	 *       malloc's  (mainly mondobuf).  So if the malloc's don't
	 *       work we just exit with zero status as long as we are
	 *       not the first child.
	 */

	nchunks = max_size / csize;
	bits = malloc((nchunks + 7) / 8);
	if (bits == 0)
		okexit(me);
	val_buf = (char *)(malloc(csize));
	if (val_buf == 0)
		okexit(me);
	zero_buf = (char *)(malloc(csize));
	if (zero_buf == 0)
		okexit(me);
	mondobuf = malloc(max_size);
	if (mondobuf == 0)
		okexit(me);

	kill(parent_pid, SIGUSR1);

	/*
	 * No init sectors; allow file to be sparse.
	 */

	val = (64 / testers) * me + 1;

	/*
	 * For each iteration:
	 *      zap bits array
	 *      loop:
	 *              pick random chunk.
	 *              if corresponding bit off {
	 *                      verify == 0. (sparse file)
	 *                      ++count;
	 *              } else
	 *                      verify == val.
	 *              write "val" on it.
	 *              repeat until count = nchunks.
	 *      ++val.
	 *      Fill-in those chunks not yet seen.
	 */

	bfill(zero_buf, 0, csize);
	bfill(mondobuf, 0, max_size);

	srand(getpid());
	while (iterations-- > 0) {
		bfill(bits, 0, (nchunks + 7) / 8);
		bfill(val_buf, val, csize);
		count = 0;
		collide = 0;
		while (count < nchunks) {
			chunk = rand() % nchunks;
			buf = mondobuf + CHUNK(chunk);

			/*
			 * If bit off, haven't seen it yet.
			 * Else, have.  Verify values.
			 */

			if ((bits[chunk / 8] & (1 << (chunk % 8))) == 0) {
				if (memcmp(buf, zero_buf, csize)) {
					tst_resm(TFAIL,
						 "\t%s[%d] bad verify @ %d (%p) for val %d count %d, should be 0x%x.\n",
						 prog, me, chunk, buf, val,
						 count, val - 1);
					tst_resm(TINFO, "\tPrev ");
					dumpbuf(buf - csize);
					dumpbuf(buf);
					tst_resm(TINFO, "\tNext ");
					dumpbuf(buf + csize);
					dumpbits(bits, (nchunks + 7) / 8);
					tst_exit();
				}
				bits[chunk / 8] |= (1 << (chunk % 8));
				++count;
			} else {
				++collide;
				if (memcmp(buf, val_buf, csize)) {
					tst_resm(TFAIL,
						 "\t%s[%d] bad verify @ %d (%p) for val %d count %d.\n",
						 prog, me, chunk, buf, val,
						 count);
					tst_resm(TINFO, "\tPrev ");
					dumpbuf(buf - csize);
					dumpbuf(buf);
					tst_resm(TINFO, "\tNext ");
					dumpbuf(buf + csize);
					dumpbits(bits, (nchunks + 7) / 8);
					tst_exit();
				}
			}

			/*
			 * Write it.
			 */

			bfill(buf, val, csize);

			if (count + collide > 2 * nchunks)
				break;
		}

		/*
		 * End of iteration, maybe before doing all chunks.
		 */
#ifdef DEBUG
		tst_resm(TINFO,
			 "\t%s[%d] val %d done, count = %d, collide = %d.\n",
			 prog, me, val, count, collide);
#endif
		for (chunk = 0; chunk < nchunks; chunk++) {
			if ((bits[chunk / 8] & (1 << (chunk % 8))) == 0)
				bfill(mondobuf + CHUNK(chunk), val, csize);
		}
		bfill(zero_buf, val, csize);
		++val;
	}

	return 0;
}

void bfill(buf, val, size)
register char *buf;
char val;
register int size;
{
	register int i;

	for (i = 0; i < size; i++)
		buf[i] = val;
}

/*
 * dumpbuf
 *	Dump the buffer.
 */

int dumpbuf(buf)
register char *buf;
{
	register int i;
	char val;
	int idx;
	int nout;

#ifdef DEBUG
	tst_resm(TINFO, "Buf: ... ");
	for (i = -10; i < 0; i++)
		tst_resm(TINFO, "%x, ", buf[i]);
	tst_resm(TINFO, "\n");
#endif

	nout = 0;
	idx = 0;
	val = buf[0];
	for (i = 0; i < csize; i++) {
		if (buf[i] != val) {
#ifdef DEBUG
			if (i == idx + 1)
				tst_resm(TINFO, "%x, ", buf[idx] & 0xff);
			else
				tst_resm(TINFO, "%d*%x, ", i - idx,
					 buf[idx] & 0xff);
#endif
			idx = i;
			val = buf[i];
			++nout;
		}
		if (nout > 10) {
#ifdef DEBUG
			tst_resm(TINFO, " ... more\n");
#endif
			return 0;
		}
	}
#ifdef DEBUG
	if (i == idx + 1)
		tst_resm(TINFO, "%x\n", buf[idx] & 0xff);
	else
		tst_resm(TINFO, "%d*%x\n", i - idx, buf[idx]);
#endif
	return 0;
}

/*
 * dumpbits
 *	Dump the bit-map.
 */

void dumpbits(bits, size)
char *bits;
register int size;
{
#ifdef DEBUG
	register char *buf;

	tst_resm(TINFO, "Bits array:");
	for (buf = bits; size > 0; --size, ++buf) {
		if ((buf - bits) % 16 == 0)
			tst_resm(TINFO, "\n%04x:\t", 8 * (buf - bits));
		tst_resm(TINFO, "%02x ", (int)*buf & 0xff);
	}
	tst_resm(TINFO, "\n");
#endif
}

/* term()
 *
 *	Parent - kill kids and return when signal arrives.
 *	Child - exit.
 */
int term()
{
#ifdef DEBUG
	tst_resm(TINFO, "\tterm -[%d]- got sig term.\n", getpid());
#endif

	if (parent_pid == getpid()) {
		massmurder();
		return 0;
	}

	exit(0);
}

int chld()
{
	if (sigset(SIGUSR1, (void (*)())chld) == SIG_ERR) {
		tst_resm(TBROK, "sigset shichld");
		exit(1);
	}
	chld_flag++;
	return 0;
}

int massmurder()
{
	int i;
	for (i = 0; i < MAXCHILD; i++) {
		if (allchild[i]) {
			kill(allchild[i], SIGTERM);
		}
	}
	return 0;
}

int okexit(me)
int me;
{
	kill(parent_pid, SIGUSR1);
	tst_resm(TINFO, "\tChild [%d] - cannot malloc buffer - exiting.\n", me);
	if (me) {
		tst_resm(TINFO, "\tThis is ok - probably swap space limit.\n");
		tst_exit();
	} else {
		tst_brkm(TBROK,
			 NULL,
			 "\tThis is not ok for first child - check parameters.\n");
	}

	return 0;
}
