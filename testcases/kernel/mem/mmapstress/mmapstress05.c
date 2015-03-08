/* IBM Corporation */
/* 01/02/2003	Port to LTP	avenkat@us.ibm.com*/
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */
/*
 *   Copyright (c) International Business Machines  Corp., 2003
 *
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

/* mfile_insque:
 *	This test mmaps a portion of a file, and then mmaps the next
 *	portion of the file in front of the virtual space containing the
 *	original mmap.  It then mmaps the preceding portion of the file behind
 *	the original mmap.  None of the mmaps can be concatenated.
 */
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
/* #include <sys/pte.h> */

/*****	LTP Port	*****/
#include "test.h"
/*****	**	**	*****/

#ifndef MMU_NARROWPTEPG
#define MMU_NARROWPTEPG	1024
#endif

/*****	LTP Port	*****/
#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
char *TCID = "mmapstress05";	//mfile_insque
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
void ok_exit();
/*****	**	**	*****/

#define ERROR(M)	(void)fprintf(stderr, "%s:  errno = %d; " M "\n", \
				progname, errno);
#define CLEAN	(void)close(fd); \
		if (munmap(mmapaddr+pagesize, pagesize) == -1) { \
			ERROR("munmap failed"); \
		} \
		if (munmap(mmapaddr, pagesize) == -1) { \
			ERROR("munmap failed"); \
		} \
		if (munmap(mmapaddr+2*pagesize, pagesize) == -1) { \
			ERROR("munmap failed"); \
		} \
		if (unlink(tmpname)) { \
			ERROR("couldn't clean up temp file"); \
		}

#define CERROR(M)	CLEAN; ERROR(M)

#define CATCH_SIG(SIG) \
        if (sigaction(SIG, &sa, 0) == -1) { \
		ERROR("couldn't catch signal " #SIG); \
                exit(1); \
        }

extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern void exit(int);

static int fd;
//static char *tmpname; 12/31/02
static char tmpname[] = "fileXXXXXX";
static char *progname;

 /*ARGSUSED*/ static
void cleanup(int sig)
{
	/*
	 * Don't check error codes - we could be signaled before the file is
	 * created.
	 */
	(void)close(fd);
	(void)unlink(tmpname);
	exit(1);
}

int main(int argc, char *argv[])
{
	size_t pagesize = (size_t) sysconf(_SC_PAGE_SIZE);
	caddr_t mmapaddr;
	char *buf;
	time_t t;
	int i;
	struct sigaction sa;

	if (!argc) {
		(void)fprintf(stderr, "argc == 0\n");
		return 1;
	}
	tst_tmpdir();
	progname = argv[0];
	(void)time(&t);
//      (void)printf("%s: Started %s", argv[0], ctime(&t)); LTP Port
	if (sbrk(pagesize - ((ulong) sbrk(0) & (pagesize - 1))) == (char *)-1) {
		ERROR("couldn't round up brk");
		anyfail();
	}
	if ((buf = sbrk(pagesize)) == (char *)-1) {
		ERROR("couldn't allocate output buffer");
		anyfail();
	}
	if ((mmapaddr = (caddr_t) sbrk(0)) == (caddr_t) - 1) {
		ERROR("couldn't find top of brk");
		anyfail();
	}

	/* i changed the second argument to NULL
	   from argv[0]. otherwise it causes the
	   open to fail
	   -- sreeni
	 */

	if ((fd = mkstemp(tmpname)) == -1) {
		ERROR("mkstemp failed");
		anyfail();
	}
	sa.sa_handler = cleanup;
	sa.sa_flags = 0;
	if (sigemptyset(&sa.sa_mask)) {
		ERROR("sigemptyset failed");
		anyfail();
	}
	CATCH_SIG(SIGINT);
	CATCH_SIG(SIGQUIT);
	CATCH_SIG(SIGTERM);
	for (i = 0; i < pagesize; i++)
		buf[i] = 'a';
	if (write(fd, buf, pagesize) != pagesize) {
		CERROR("couldn't write page case 1");
		anyfail();
	}
	if (lseek(fd, MMU_NARROWPTEPG * pagesize, SEEK_SET) == -1) {
		CERROR("lseek case 1 failed");
		anyfail();
	}
	if (write(fd, buf, pagesize) != pagesize) {
		CERROR("couldn't write page case 2");
		anyfail();
	}
	if (lseek(fd, 2 * MMU_NARROWPTEPG * pagesize, SEEK_SET) == -1) {
		CERROR("lseek case 2 failed");
		anyfail();
	}
	if (write(fd, buf, pagesize) != pagesize) {
		CERROR("couldn't write page case 3");
		anyfail();
	}
	/* fd now references a sparce file which has three pages widely spaced.
	 * Hopefully different mfile objects will be needed to reference each
	 * page.
	 */
	if (mmap(mmapaddr + pagesize, pagesize, PROT_READ,
		 MAP_FILE | MAP_PRIVATE | MAP_FIXED, fd,
		 MMU_NARROWPTEPG * pagesize)
	    == (caddr_t) - 1) {
		CERROR("first mmap (of third page) failed");
		anyfail();
	}
	if (mmap(mmapaddr, pagesize, PROT_READ,
		 MAP_FILE | MAP_PRIVATE | MAP_FIXED, fd,
		 2 * MMU_NARROWPTEPG * pagesize)
	    == (caddr_t) - 1) {
		CERROR("second mmap (of fifth page) failed");
		anyfail();
	}
	if (mmap(mmapaddr + 2 * pagesize, pagesize, PROT_READ,
		 MAP_FILE | MAP_PRIVATE | MAP_FIXED, fd, 0) == (caddr_t) - 1) {
		CERROR("third mmap (of first page) failed");
		anyfail();
	}
	CLEAN;			/*comment */
	(void)time(&t);
//      (void)printf("%s: Finished %s", argv[0], ctime(&t)); LTP Port
	ok_exit();
	tst_exit();
}

void ok_exit(void)
{
	tst_resm(TPASS, "Test passed\n");
	tst_rmdir();
	tst_exit();
}

int anyfail(void)
{
	tst_brkm(TFAIL, tst_rmdir, "Test failed\n");
}
