/* IBM Corporation */
/* 01/02/2003	Port to LTP avenkat@us.ibm.com */
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

/* uiomove_phys_fail:
 *	Test a copyout/copyin failure in the kernel primitive uiomove_phys by
 *	reading into or writing from a mmaped regular file which lacks the
 *	needed permissions.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

extern time_t time(time_t *);
extern char *ctime(const time_t *);
extern void exit(int);

#define	ERROR(M)	(void)fprintf(stderr, "%s: errno = %d; " M "\n", \
				argv[0], errno)
#define CLEANERROR(M)	(void)unlink(tmpname); ERROR(M)
#define CATCH_SIG(SIG) \
        if (sigaction(SIG, &sa, 0) == -1) { \
                ERROR("couldn't catch signal " #SIG); \
                exit(1); \
        }

static char tmpname[] = "fileXXXXXX";
static int fd;
/*****  LTP Port        *****/
#include "test.h"
#define FAILED 0
#define PASSED 1

int local_flag = PASSED;
char *TCID = "mmapstress02";	//uiomove_phys_fail
FILE *temp;
int TST_TOTAL = 1;

int anyfail();
void ok_exit();
/*****  **      **      *****/

 /*ARGSUSED*/ static
void cleanup(int sig)
{
	/*
	 * Don't check error codes - we could be signaled before the file is
	 * created.
	 */
	(void)close(fd);
	(void)unlink(tmpname);
	tst_rmdir();
	tst_exit();
}

int main(int argc, char *argv[])
{
	caddr_t mmapaddr;
	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	time_t t;
	int i;
	struct sigaction sa;

	tst_tmpdir();
	if (!argc) {
		(void)fprintf(stderr, "argc == 0\n");
		return 1;
	}
	if (argc != 1) {
		(void)fprintf(stderr, "usage: %s\n", argv[0]);
		return 1;
	}
	(void)time(&t);
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
	if (sbrk(2 * pagesize - ((ulong) sbrk(0) & (pagesize - 1))) ==
	    (char *)-1) {
		CLEANERROR("couldn't round up brk");
		anyfail();
	}
	if ((mmapaddr = sbrk(0)) == (caddr_t) - 1) {
		CLEANERROR("couldn't find top of brk");
		anyfail();
	}
	/* Write a page of garbage into the file, so we can mmap it without
	 * asking for PROT_WRITE.
	 */
	for (i = pagesize; i; i--)
		*(mmapaddr - i) = 'a';
	if (write(fd, (char *)mmapaddr - pagesize, pagesize) != pagesize) {
		CLEANERROR("write failed");
		anyfail();
	}
	if (mmap(mmapaddr, pagesize, PROT_NONE,
		 MAP_FIXED | MAP_PRIVATE | MAP_FILE, fd, 0) != mmapaddr) {
		CLEANERROR("couldn't mmap file");
		anyfail();
	}
	/*
	 * Since the file is mmapped, mmreg_new and uiomove_phys handle all
	 * I/O
	 */
	if (lseek(fd, 0, SEEK_SET) != 0) {
		CLEANERROR("lseek failed");
		anyfail();
	}
	if (read(fd, (char *)mmapaddr, pagesize) != -1) {
		CLEANERROR("read succeded");
		anyfail();
	}
	if (errno != EFAULT) {
		CLEANERROR("read didn't set errno = EFAULT");
		anyfail();
	}
	if (write(fd, (char *)mmapaddr, pagesize) != -1) {
		CLEANERROR("write succeded");
		anyfail();
	}
	if (errno != EFAULT) {
		CLEANERROR("write didn't set errno = EFAULT");
		anyfail();
	}
	if (close(fd) == -1) {
		CLEANERROR("close failed");
		anyfail();
	}
	if (munmap(mmapaddr, pagesize) == -1) {
		CLEANERROR("munmap failed");
		anyfail();
	}
	if (unlink(tmpname) == -1) {
		ERROR("unlink failed");
		anyfail();
	}
	(void)time(&t);
//      (void)printf("%s: Finished %s", argv[0], ctime(&t));
	ok_exit();		/* LTP Port */
	tst_exit();
}

/*****  LTP Port        *****/
void ok_exit(void)
{
	tst_resm(TPASS, "Test passed\n");
	tst_rmdir();
	tst_exit();
}

int anyfail(void)
{
	tst_brkm(TFAIL, tst_rmdir, "Test failed");
}

/*****  **      **      *****/
