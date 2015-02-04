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

/* 11/19/2002	Port to LTP	robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

/*
 * NAME
 *	stack_space.c - stack test
 *
 *	Test VM for set of stack-space intensive programs.
 *	This code very similar to tdat.c, only uses stack-based "file".
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/** LTP Port **/
#include "test.h"

#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
int block_number;

char *TCID = "stack_space";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
/**************/

#define MAXCHILD	100	/* max # kids */
#define K_1		1024
#define K_2		2048
#define K_4		4096
#define MAXSIZE         10*K_1

int nchild;			/* # kids */
int csize;			/* chunk size */
int iterations;			/* # total iterations */
int parent_pid;

int usage(char *);
int bd_arg(char *);
int runtest();
int dotest(int, int);
int bfill(char *, char, int);
int dumpbuf(char *);
void dumpbits(char *, int);

char *prog;			/* invoked name */

int usage(char *prog)
{
	tst_resm(TCONF, "Usage: %s <nchild> <chunk_size> <iterations>", prog);
	tst_brkm(TCONF, NULL, "DEFAULTS: 20 1024 50");
}

int main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	void term();

	prog = argv[0];
	parent_pid = getpid();

	if (signal(SIGTERM, term) == SIG_ERR) {
		tst_brkm(TBROK, NULL, "first sigset failed");

	}

	if (argc == 1) {
		nchild = 20;
		csize = K_1;
		iterations = 50;
	} else if (argc == 4) {
		i = 1;
		if (sscanf(argv[i++], "%d", &nchild) != 1)
			bd_arg(argv[i - 1]);
		if (nchild > MAXCHILD) {
			tst_brkm(TBROK, NULL,
				 "Too many children, max is %d\n",
				 MAXCHILD);
		}
		if (sscanf(argv[i++], "%d", &csize) != 1)
			bd_arg(argv[i - 1]);
		if (csize > MAXSIZE) {
			tst_brkm(TBROK, NULL,
				 "Chunk size too large , max is %d\n",
				 MAXSIZE);
		}
		if (sscanf(argv[i++], "%d", &iterations) != 1)
			bd_arg(argv[i - 1]);
	} else
		usage(prog);

	tst_tmpdir();
	runtest();
	/**NOT REACHED**/
	return 0;

}

int bd_arg(str)
char *str;
{
	tst_brkm(TCONF, NULL,
		 "Bad argument - %s - could not parse as number.\n",
		 str);
}

int runtest()
{
	register int i;
	int child;
	int status;
	int count;

	for (i = 0; i < nchild; i++) {
		if ((child = fork()) == 0) {	/* child */
			dotest(nchild, i);	/* do it! */
			exit(0);	/* when done, exit */
		}
		if (child < 0) {
			tst_resm(TBROK,
				 "Fork failed (may be OK if under stress)");
			tst_resm(TINFO, "System resource may be too low.\n");
			tst_brkm(TBROK, tst_rmdir, "Reason: %s\n",
				 strerror(errno));

		}
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
			tst_resm(TINFO, "\tFailed - expected 0 exit status.\n");
			local_flag = FAILED;
		}
		++count;
	}

	/*
	 * Should have collected all children.
	 */

	if (count != nchild) {
		tst_resm(TINFO, "\tWrong # children waited on, count = %d\n",
			 count);
		local_flag = FAILED;
	}

	(local_flag == FAILED) ? tst_resm(TFAIL, "Test failed")
	    : tst_resm(TPASS, "Test passed");
	sync();			/* safeness */
	tst_rmdir();
	tst_exit();

}

/*
 * dotest()
 *	Children execute this.
 *
 * Randomly read/mod/write chunks with known pattern and check.
 * When fill sectors, iterate.
 */

int nchunks;

#define	CHUNK(i)	((i) * csize)

int dotest(int testers, int me)
{
	char *bits;
	char *val_buf;
	char *zero_buf;
	char *buf;
	int count;
	int collide;
	char val;
	int chunk;
	char mondobuf[MAXSIZE];

	nchunks = MAXSIZE / csize;
	bits = malloc((nchunks + 7) / 8);
	val_buf = (char *)(malloc(csize));
	zero_buf = (char *)(malloc(csize));

	if (bits == 0 || val_buf == 0 || zero_buf == 0) {
		tst_brkm(TFAIL, NULL, "\tmalloc failed, pid: %d\n", getpid());
	}

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
	bfill(mondobuf, 0, MAXSIZE);

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
						 "%s[%d] bad verify @ %d (%p) for val %d count %d, should be 0.\n",
						 prog, me, chunk, buf, val,
						 count);
					tst_resm(TINFO, "Prev ");
					dumpbuf(buf - csize);
					dumpbuf(buf);
					tst_resm(TINFO, "Next ");
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
						 "%s[%d] bad verify @ %d (%p) for val %d count %d.\n",
						 prog, me, chunk, buf, val,
						 count);
					tst_resm(TINFO, "Prev ");
					dumpbuf(buf - csize);
					dumpbuf(buf);
					tst_resm(TINFO, "Next ");
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

		for (chunk = 0; chunk < nchunks; chunk++) {
			if ((bits[chunk / 8] & (1 << (chunk % 8))) == 0)
				bfill(mondobuf + CHUNK(chunk), val, csize);
		}
		bfill(zero_buf, val, csize);
		++val;
	}
	free(bits);
	free(val_buf);
	free(zero_buf);

	return 0;
}

int bfill(buf, val, size)
register char *buf;
char val;
register int size;
{
	register int i;

	for (i = 0; i < size; i++)
		buf[i] = val;
	return 0;
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

void term()
{

	if (getpid() == parent_pid) {
#ifdef DEBUG
		tst_resm(TINFO, "term - parent - got SIGTERM.\n");
#endif
	} else {
#ifdef DEBUG
		tst_resm(TINFO, "term1 - child - exiting\n");
#endif
		exit(0);
	}
}
