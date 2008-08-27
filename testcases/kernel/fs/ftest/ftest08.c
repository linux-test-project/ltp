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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	ftest08.c -- test single file io (tsfio.c by rbk) (ported from SPIE,
 *		     section2/filesuite/ftest10.c, by Airong Zhang)
 *
 * 	this is the same as ftest5, except that it uses lseek64
 *
 * CALLS
 *	fsync, sync, lseek64, read, write
 *	
 *
 * ALGORITHM
 *	Several child processes doing random seeks, read/write
 *	operations on the same file.
 *
 *
 * RESTRICTIONS
 *	Runs a long time with default args - can take others on input
 *	line.  Use with "term mode".
 *	
 */

#define _XOPEN_SOURCE 500
#define  _LARGEFILE64_SOURCE 1
#include <stdio.h>		/* needed by testhead.h		*/
#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <errno.h>
#include <signal.h>		/* DEM - added SIGTERM support */
#include <unistd.h>
#include "test.h"
#include "usctest.h"

char *TCID = "ftest08";
int TST_TOTAL = 1;
extern int Tst_count;

#define PASSED 1
#define FAILED 0

#define MAXCHILD	25	/* max number of children to allow */
#define K_1		1024
#define K_2		2048
#define K_4		4096
#define	MAXIOVCNT	16

void init();
int runtest();
int dotest(int, int, int);
int domisc(int, int, char*);
int bfill(char*, char, int);
int dumpiov(struct iovec*);
int dumpbits(char*, int);
int term();
void cleanup(void);

int	csize;				/* chunk size */
int	iterations;			/* # total iterations */
off64_t max_size;			/* max file size */
int	misc_intvl;			/* for doing misc things; 0 ==> no */
int	nchild;				/* number of child processes */
int	nwait;
int	parent_pid;
int	pidlist[MAXCHILD];

char	filename[128];
char	*prog;

int	local_flag;

/*--------------------------------------------------------------*/
int main (ac, av)
	int  ac;
	char *av[];
{
        int lc;                 /* loop counter */
        char *msg;              /* message returned from parse_opts */

        /*
         * parse standard options
         */
        if ((msg = parse_opts(ac, av, (option_t *)NULL, NULL)) != (char *)NULL){
                tst_resm(TBROK, "OPTION PARSING ERROR - %s", msg);
                tst_exit();
                /*NOTREACHED*/
        }

	for (lc = 0; TEST_LOOPING(lc); lc++) {


		local_flag = PASSED;

		init();

		runtest();

		if (local_flag == PASSED) {
                        tst_resm(TPASS, "Test passed.");
                } else {
                        tst_resm(TFAIL, "Test failed.");
                }

	} /* end for */
	cleanup();
	return(0);
}

void init()
{
	int fd;
	char wdbuf[MAXPATHLEN];

	parent_pid = getpid();
	tst_tmpdir();
	/*
	 * Make a filename for the test.
	 */

	if (!filename[0])
		sprintf(filename, "%s/ftest08.%d", getcwd(wdbuf, MAXPATHLEN), getpid());

	fd = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0666);
	if (fd < 0) {
		tst_resm(TBROK, "Error %d creating file %s", errno, filename);
		tst_exit();
	}
	close(fd);

	/*
	 * Default values for run conditions.
	 */

	iterations = 10;
	nchild = 5;
	csize = K_2;		/* should run with 1, 2, and 4 K sizes */
	max_size = K_1 * K_1;
	misc_intvl = 10;

	if (sigset(SIGTERM, (void (*)())term) == SIG_ERR) {
		tst_resm(TBROK,"first sigset failed");
		tst_exit();
	}

}

/*--------------------------------------------------------------*/


int runtest()
{
	register int i;
	int	child;
	int	status;
	int	count;
	int	fd;


	for(i = 0; i < nchild; i++) {
		if ((child = fork()) == 0) {		/* child */
			fd = open(filename, O_RDWR);
			if (fd < 0) {
				tst_resm(TFAIL, "\tTest[%d]: error %d openning %s.", errno, filename);
				tst_exit();
			}
			dotest(nchild, i, fd);		/* do it! */
			close(fd);
			tst_exit();			/* when done, exit */
		}
		if (child < 0) {
			tst_resm(TINFO, "System resource may be too low, fork() malloc()"
		                            " etc are likely to fail.");
		        tst_resm(TBROK, "Test broken due to inability of fork.");
		        tst_exit();

		} else {
			pidlist[i] = child;
			nwait++;
		}
	}

	/*
	 * Wait for children to finish.
	 */

	count = 0;
	while((child = wait(&status)) != -1 || errno == EINTR) {
		if (child > 0)
		{
			//tst_resm(TINFO, "\tTest{%d} exited status = 0x%x", child, status);
			if (status) {
				tst_resm(TFAIL, "\tExpected 0 exit status - failed.");
				local_flag = FAILED;
			}
			++count;
		}
	}

	/*
	 * Should have collected all children.
	 */

	if (count != nwait) {
		tst_resm(TFAIL, "\tWrong # children waited on, count = %d", count);
		local_flag = FAILED;
	}

	unlink(filename);

	sync();				/* safeness */
	return(0);
}

/*
 * dotest()
 *	Children execute this.
 *
 * Randomly read/mod/write chunks with known pattern and check.
 * When fill sectors, iterate.
 */

#define	NMISC	2
enum	m_type { m_fsync, m_sync };
char	*m_str[] = {
		"fsync",   "sync"
};

int	misc_cnt[NMISC];		/* counts # of each kind of misc */
int	misc_flag;
int	nchunks;

#define	CHUNK(i)	((((off64_t)i) * testers + me) * csize)
#define	NEXTMISC	((rand() % misc_intvl) + 5)

int dotest(testers, me, fd)
	int	testers;
	int	me;
	int	fd;
{
	register int	i;
	char	*bits;
	char	*buf;
	int	count;
	int	collide;
	char	val;
	char	val0;
	int	chunk;
	int	whenmisc;
	int	xfr;

	/* Stuff for the readv call */
	struct	iovec	r_iovec[MAXIOVCNT];
	int	r_ioveclen;

	/* Stuff for the writev call */
							
	struct	iovec	val0_iovec[MAXIOVCNT];
	struct	iovec	val_iovec[MAXIOVCNT];
	int	w_ioveclen;

	nchunks = max_size / (testers * csize);
	if( (bits = (char*)malloc((nchunks+7)/8)) == 0) {
		tst_resm(TBROK, "\tmalloc failed(bits)");
		tst_exit();
	}
	if( (buf = (char*)(malloc(csize))) == 0) {
		tst_resm(TBROK, "\tmalloc failed(buf)");
		tst_exit();
	}
	
	/*Allocate memory for the iovec buffers and init the iovec arrays
	 */
	r_ioveclen = w_ioveclen = csize / MAXIOVCNT;

		/* Please note that the above statement implies that csize
		 * be evenly divisible by MAXIOVCNT.
		 */

	for (i = 0; i < MAXIOVCNT; i++) {
		if( (r_iovec[i].iov_base = (char*)malloc(r_ioveclen)) == 0) {
			tst_resm(TBROK, "\tmalloc failed(iov_base)");
			tst_exit();
		}
		r_iovec[i].iov_len = r_ioveclen;

		/* Allocate unused memory areas between all the buffers to
		 * make things more diffult for the OS.
		 */

		if(malloc((i+1)*8) == 0) {
			tst_resm(TBROK, "\tmalloc failed((i+1)*8)");
			tst_exit();
		}
		if( (val0_iovec[i].iov_base = (char*)malloc(w_ioveclen)) == 0){
			tst_resm(TBROK, "\tmalloc failed(val0_iovec)");
			tst_exit();
		}
		val0_iovec[i].iov_len = w_ioveclen;
		
		if(malloc((i+1)*8) == 0) {
			tst_resm(TBROK, "\tmalloc failed((i+1)*8)");
			tst_exit();
		}
		if( (val_iovec[i].iov_base = (char*)malloc(w_ioveclen)) == 0){
			tst_resm(TBROK, "\tmalloc failed(iov_base)");
			tst_exit();
		}
		val_iovec[i].iov_len = w_ioveclen;

		if(malloc((i+1)*8) == 0) {
			tst_resm(TBROK, "\tmalloc failed(((i+1)*8)");
			tst_exit();
		}
	}

	/*
	 * No init sectors; file-sys makes 0 to start.
	 */

	val = (64/testers) * me + 1;
	val0 = 0;

	/*
	 * For each iteration:
	 *	zap bits array
	 *	loop:
	 *		pick random chunk, read it.
	 *		if corresponding bit off {
	 *			verify == 0. (sparse file)
	 *			++count;
	 *		} else
	 *			verify == val.
	 *		write "val" on it.
	 *		repeat until count = nchunks.
	 *	++val.
	 */

	srand(getpid());
	if (misc_intvl) whenmisc = NEXTMISC;
	while(iterations-- > 0) {
		for(i = 0; i < NMISC; i++)
			misc_cnt[i] = 0;
		bfill(bits, 0, (nchunks+7)/8);
		/* Have to fill the val0 and val iov buffers in a different manner
		 */
		for(i = 0; i < MAXIOVCNT; i++) {
			bfill(val0_iovec[i].iov_base,val0,val0_iovec[i].iov_len);
			bfill(val_iovec[i].iov_base,val,val_iovec[i].iov_len);

		}
		count = 0;
		collide = 0;
		while(count < nchunks) {
			chunk = rand() % nchunks;
			/*
			 * Read it.
			 */
			if (lseek64(fd, CHUNK(chunk), 0) < (off64_t)0) {
				tst_resm(TFAIL, "\tTest[%d]: lseek64(0) fail at %Lx, errno = %d.",
					me, CHUNK(chunk), errno);
				tst_exit();
			}
			if ((xfr = readv(fd, &r_iovec[0], MAXIOVCNT)) < 0) {
				tst_resm(TFAIL, "\tTest[%d]: readv fail at %Lx, errno = %d.",
					me, CHUNK(chunk), errno);
				tst_exit();
			}
			/*
			 * If chunk beyond EOF just write on it.
			 * Else if bit off, haven't seen it yet.
			 * Else, have.  Verify values.
			 */
			if (xfr == 0) {
				bits[chunk/8] |= (1<<(chunk%8));
			} else if ((bits[chunk/8] & (1<<(chunk%8))) == 0) {
				if (xfr != csize) {
					tst_resm(TFAIL, "\tTest[%d]: xfr=%d != %d, zero read.",
						me, xfr, csize);
					tst_exit();
				}
				for(i = 0; i < MAXIOVCNT; i++) {
					if (memcmp(r_iovec[i].iov_base, val0_iovec[i].iov_base, r_iovec[i].iov_len)) {
						tst_resm(TFAIL, "\tTest[%d] bad verify @ 0x%Lx for val %d count %d xfr %d.",
							me, CHUNK(chunk), val0, count, xfr);
						dumpiov(&r_iovec[i]);
						dumpbits(bits, (nchunks+7)/8);
						tst_exit();
					}
				}
				bits[chunk/8] |= (1<<(chunk%8));
				++count;
			} else {
				if (xfr != csize) {
					tst_resm(TFAIL, "\tTest[%d]: xfr=%d != %d, val read.",
						me, xfr, csize);
					tst_exit();
				}
				++collide;
				for(i = 0; i < MAXIOVCNT; i++) {
					if (memcmp(r_iovec[i].iov_base, val_iovec[i].iov_base, r_iovec[i].iov_len)) {
						tst_resm(TFAIL, "\tTest[%d] bad verify @ 0x%Lx for val %d count %d xfr %d.",
							me, CHUNK(chunk), val, count, xfr);
						dumpiov(&r_iovec[i]);
						dumpbits(bits, (nchunks+7)/8);
						tst_exit();
					}
				}
			}
			/*
			 * Write it.
			 */
			if (lseek64(fd, -((off64_t)xfr), 1) <  (off64_t)0) {
				tst_resm(TFAIL, "\tTest[%d]: lseek64(1) fail at %Lx, errno = %d.",
					me, CHUNK(chunk), errno);
				tst_exit();
			}
			if ((xfr = writev(fd, &val_iovec[0], MAXIOVCNT)) < csize) {
				if (errno == ENOSPC) {
					tst_resm(TFAIL, "\tTest[%d]: no space, exiting.", me);
					fsync(fd);
					tst_exit();
				}
				tst_resm(TFAIL, "\tTest[%d]: writev fail at %Lx xfr %d, errno = %d.",
					me, CHUNK(chunk), xfr, errno);
				tst_exit();
			}
			/*
			 * If hit "misc" interval, do it.
			 */
			if (misc_intvl && --whenmisc <= 0) {
				domisc(me, fd, bits);
				whenmisc = NEXTMISC;
			}
			if (count + collide > 2 * nchunks)
				break;
		}

		/*
		 * End of iteration, maybe before doing all chunks.
		 */

		if (count < nchunks) {
			//tst_resm(TINFO, "\tTest{%d} val %d stopping @ %d, collide = {%d}.",
			//		me, val, count, collide);
			for(i = 0; i < nchunks; i++) {
				if ((bits[i/8] & (1<<(i%8))) == 0) {
					if (lseek64(fd, CHUNK(i), 0) < (off64_t)0) {
						tst_resm(TFAIL, "\tTest[%d]: lseek64 fail at %Lx, errno = %d.",
							me, CHUNK(i), errno);
						tst_exit();
					}
					if (writev(fd, &val_iovec[0], MAXIOVCNT) != csize) {
						tst_resm(TFAIL, "\tTest[%d]: writev fail at %Lx, errno = %d.",
							me, CHUNK(i), errno);
						tst_exit();
					}
				}
			}
		}

		fsync(fd);
		++misc_cnt[(int)m_fsync];
		//tst_resm(TINFO, "\tTest[%d] val %d done, count = %d, collide = %d.",
		//		me, val, count, collide);
		//for(i = 0; i < NMISC; i++)
		//	tst_resm(TINFO, "\t\tTest[%d]: %d %s's.", me, misc_cnt[i], m_str[i]);
		val0 = val++;
	}
	return(0);
}

/*
 * domisc()
 *	Inject misc syscalls into the thing.
 */

int domisc(me, fd, bits)
	int	me;
	int	fd;
	char	*bits;
{
	enum	m_type	type;

	if (misc_flag) {
		type = m_fsync;
		misc_flag = 0;
	} else {
		type = m_sync;;
		misc_flag = 1;
	}
	switch(type) {
	case m_fsync:
		if (fsync(fd) < 0) {
			tst_resm(TFAIL, "\tTest[%d]: fsync error %d.", me, errno);
			tst_exit();
		}
		break;
	case m_sync:
		sync();
		break;
	}
	++misc_cnt[(int)type];
	return(0);
}

int bfill(buf, val, size)
	register char *buf;
	char	val;
	register int size;
{
	register int i;

	for(i = 0; i < size; i++)
		buf[i] = val;
	return(0);
}

/*
 * dumpiov
 *	Dump the contents of the r_iovec buffer.
 */

int dumpiov(iovptr)
	register struct iovec *iovptr;
{
	register int i;
	char	val;
	int	idx;
	int	nout;

	tst_resm(TINFO, "\tBuf:");
	nout = 0;
	idx = 0;
	val = ((char *)iovptr->iov_base)[0];
	for(i = 0; i < iovptr->iov_len; i++) {
		if (((char *)iovptr->iov_base)[i] != val) {
			if (i == idx+1)
				tst_resm(TINFO, "\t%x, ", ((char *)iovptr->iov_base)[idx] & 0xff);
			else
				tst_resm(TINFO, "\t%d*%x, ", i-idx, ((char *)iovptr->iov_base)[idx] & 0xff);
			idx = i;
			++nout;
		}
		if (nout > 10) {
			tst_resm(TINFO, "\t ... more");
			return(0);
		}
	}
	if (i == idx+1)
		tst_resm(TINFO, "\t%x", ((char *)iovptr->iov_base)[idx] & 0xff);
	else
		tst_resm(TINFO, "\t%d*%x", i-idx, ((char *)iovptr->iov_base)[idx]);
	return(0);
}


/*
 * dumpbits
 *	Dump the bit-map.
 */

int dumpbits(bits, size)
	char	*bits;
	register int size;
{
	register char *buf;

	tst_resm(TINFO, "\tBits array:");
	for(buf = bits; size > 0; --size, ++buf) {
		if ((buf-bits) % 16 == 0)
			tst_resm(TINFO, "\t%04x:\t", 8*(buf-bits));
		tst_resm(TINFO, "%02x ", (int)*buf & 0xff);
	}
	printf("\n");
	return(0);
}

/*--------------------------------------------------------------*/


int term()
{
	register int i;

	tst_resm(TINFO, "\tterm -[%d]- got sig term.", getpid());

	if (parent_pid == getpid()) {
		for (i=0; i < nchild; i++)
			if (pidlist[i])		/* avoid embarassment */
				kill(pidlist[i], SIGTERM);
		return(0);
	}

	tst_exit();
	return(0);
}

void
cleanup()
{
        /*
         * print timing stats if that option was specified.
         * print errno log if that option was specified.
         */
        TEST_CLEANUP;

        tst_rmdir();
        tst_exit();
}

