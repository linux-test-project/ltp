/* IBM Corporation */
/* 01/02/2003	Port to LTP avenkat@us.ibm.com	*/
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * weave:
 *	Mmap parts of a file, and then write to the file causing single
 *	write requests to jump back and forth between mmaped io and regular io.
 *
 *	Usage: weave filename startoffset. pageoffset = 4096.
 *
 *	startoffset specifies a byte count in the file at which to begin
 *	the test.  When this value is non-zero, a sparse file is created.
 *	This is useful for testing with large files.
 *
 *  Compile with -DLARGE_FILE to enable file sizes > 2 GB.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
/*****	LTP Port	*****/
#include "test.h"
#include "usctest.h"
#define FAILED 0
#define PASSED 1
/*****	**	**	*****/

#define CLEAN	(void)close(rofd); \
		(void)close(rwfd); \
		(void)unlink(filename);
#define ERROR(M)	(void)fprintf(stderr, "%s:  errno = %d; " M "\n", \
				argv[0], errno)
#define CLEANERROR(M)	CLEAN; ERROR(M)
#define CATCH_SIG(SIG) \
        if (sigaction(SIG, &sa, 0) == -1) { \
                ERROR("couldn't catch signal " #SIG); \
                exit(1); \
        }

static char *filename;

/*****	LTP Port	*****/
char *TCID = "mmapstress04";//weave
int local_flag = PASSED;
int block_number;
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
int blenter();
int blexit();
int instress();
void setup();
void terror();
void fail_exit();
void ok_exit();
/*****	**	**	*****/

extern time_t	time(time_t *);
extern char	*ctime(const time_t *);
extern void     exit(int);
static int	rofd, rwfd;

/*ARGSUSED*/
static
void
cleanup(int sig)
{
        /*
         * Don't check error codes - we could be signaled before the file is
         * created.
         */
        (void)close(rofd);
        (void)close(rwfd);
        (void)unlink(filename);
        exit(1);
}

int
main(int argc, char *argv[])
{
	char 			*buf;
	size_t			pagesize = (size_t)sysconf(_SC_PAGE_SIZE);
	caddr_t 		mmapaddr;
	time_t			t;
	int			i, j;
        struct sigaction        sa;
#ifdef LARGE_FILE
	off64_t			startoffset;
	off64_t			seekoff;
	off64_t			mapoff;
#else /* LARGE_FILE */
	off_t			startoffset;
	off_t			seekoff;
	off_t			mapoff;
#endif /* LARGE_FILE */

	if (argc < 2 || argc > 3) {
		(void)fprintf(stderr, "Usage: %s filename startoffset\n",
			      argv[0]);
		return 1;
	}
	filename = argv[1];

	if (argc >= 3) {
#ifdef LARGE_FILE
		startoffset = atoll(argv[2]);
#else /* LARGE_FILE */
		startoffset = atoi(argv[2]);
#endif /* LARGE_FILE */
	}
	else
		startoffset = pagesize;

	if (startoffset % pagesize != 0) {
		fprintf(stderr, "pagesize=%ld\n", (long)pagesize);
		fprintf(stderr, "startoffset must be a pagesize multiple\n");
		anyfail();  //LTP Port
	}
	(void)time(&t);
//	(void)printf("%s: Started %s", argv[0], ctime(&t));
	if ((buf = sbrk(6*pagesize)) == (char *)-1) {
		ERROR("couldn't allocate buf");
		anyfail();	//LTP Port
	}
	if (sbrk(pagesize-((ulong)sbrk(0)&(pagesize-1))) == (char *)-1) {
		ERROR("couldn't round up brk");
		anyfail();	//LTP Port
	}
	if ((mmapaddr = (caddr_t)sbrk(0)) == (caddr_t)-1) {
		ERROR("couldn't find top of brk");
		anyfail();	//LTP Port
	}
	sa.sa_handler = cleanup;
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask)) {
		ERROR("sigemptyset failed");
		anyfail();	//LTP Port
	}
	CATCH_SIG(SIGINT);
	CATCH_SIG(SIGQUIT);
	CATCH_SIG(SIGTERM);
	tst_tmpdir();
#ifdef LARGE_FILE
	if ((rofd = open64(filename, O_RDONLY|O_CREAT, 0777)) == -1) {
#else /* LARGE_FILE */
	if ((rofd = open(filename, O_RDONLY|O_CREAT, 0777)) == -1) {
#endif /* LARGE_FILE */
		ERROR("read only open failed");
		anyfail();	//LTP Port
	}
#ifdef LARGE_FILE
	if ((rwfd = open64(filename, O_RDWR)) == -1) {
#else /* LARGE_FILE */
	if ((rwfd = open(filename, O_RDWR)) == -1) {
#endif /* LARGE_FILE */
		(void)close(rofd);
		(void)unlink(filename);
		ERROR("read/write open failed");
		anyfail();	//LTP Port
	}
#ifdef LARGE_FILE
	seekoff = startoffset + (off64_t)64 * (off64_t)6 * (off64_t)pagesize;
	if (lseek64(rwfd, seekoff, SEEK_SET) != seekoff) {
#else /* LARGE_FILE */
	seekoff = startoffset + (off_t)64 * (off_t)6 * (off_t)pagesize;
	if (lseek(rwfd, seekoff, SEEK_SET) != seekoff) {
#endif /* LARGE_FILE */
		CLEANERROR("first lseek failed");
		anyfail();	//LTP Port
	}
	i = 0;
	while (i < pagesize && write(rwfd, "b", 1) == 1)
		i++;
	if (i != pagesize) {
		CLEANERROR("write to extend file failed");
		anyfail();	//LTP Port
	}
	/* The file is now really big, and empty.
	 * Assuming disk blocks are 8k, and logical pages are 4k, there are
	 * two maps per page.  In order to test mapping at the beginning and
	 * ends of the block, mapping the whole block, or none of the block
	 * with different mappings on preceding and following blocks, each
	 * 3 blocks with 6 pages can be thought of as a binary number from 0 to
	 * 64 with a bit set for mapped or cleared for unmapped.  This number
	 * is represented by i.  The value j is used to look at the bits of i
	 * and decided to map the page or not.
	 * NOTE: None of the above assumptions are critical.
	 */
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 6; j++) {
			if (i & (1<<j)) {
#ifdef LARGE_FILE
				mapoff = startoffset +
					 (off64_t)pagesize * (off64_t)(6+i+j);
				if (mmap64(mmapaddr+pagesize*(6*i+j),
					pagesize, PROT_READ,
					MAP_FILE|MAP_PRIVATE|MAP_FIXED, rofd,
					mapoff)
				        == (caddr_t)-1) {
#else /* LARGE_FILE */
				mapoff = startoffset +
					 (off_t)pagesize * (off_t)(6+i+j);
				if (mmap(mmapaddr+pagesize*(6*i+j),
					pagesize, PROT_READ,
					MAP_FILE|MAP_PRIVATE|MAP_FIXED, rofd,
					mapoff)
				        == (caddr_t)-1) {
#endif /* LARGE_FILE */
					CLEANERROR("mmap failed");
					anyfail();	//LTP Port
				}
			}
		}
	}
	/* done mapping */
	for (i = 0; i < 6*pagesize; i++)
		buf[i] = 'a';
	/* write out 6 pages of stuff into each of the 64 six page sections */
#ifdef LARGE_FILE
	if (lseek64(rwfd, startoffset,  SEEK_SET) != startoffset) {
#else /* LARGE_FILE */
	if (lseek(rwfd, startoffset, SEEK_SET) != startoffset) {
#endif /* LARGE_FILE */
		CLEANERROR("second lseek failed");
		anyfail();	//LTP Port
	}
	for (i = 0; i < 64; i++) {
		if (write(rwfd, buf, 6*pagesize) != 6*pagesize) {
			CLEANERROR("write failed");
			anyfail();	//LTP Port
		}
	}
	/* Just finished scribbling all over interwoven mmapped and unmapped
	 * regions.
	 */
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 6; j++) {
			/* if mmaped && not updated */
			if ((i & (1<<j)) && *(mmapaddr+pagesize*(6*i+j))!='a')
			{
				CLEANERROR("'a' missing from mmap");
				(void)fprintf(stderr, "i=%d\nj=%d\n"
					"val=0x%x\n", i, j,
					(int)(*(mmapaddr+pagesize*(6*i+j))));
				anyfail();	//LTP Port
			}
		}
	}
	/* Just checked to see that each mmapped page at least had an 'a' at
	 * the beginning.
	 */
	CLEAN;
	(void)time(&t);
//	(void)printf("%s: Finished %s", argv[0], ctime(&t)); LTP Port
   	(local_flag == FAILED) ? tst_resm(TFAIL, "Test failed\n") : tst_resm(TPASS, "Test passed\n"); //LTP Port
  	tst_rmdir();
   	tst_exit();	//LTP Port

	tst_exit();
}

/*****	LTP Port	*****/
int anyfail()
{
  tst_resm(TFAIL, "Test failed\n");
  tst_rmdir();
  tst_exit();
        return 0;
}

/*****	**	**	*****/