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
 *      diotest4.c
 *
 * DESCRIPTION
 *	The program generates error conditions and verifies the error
 *	code generated with the expected error value. The program also 
 *	tests some of the boundary condtions. The size of test file created
 *	is filesize_in_blocks * 4k.
 *	Test blocks:
 *	[1] Negative Offset 
 *	[2] Negative count - removed 08/01/2003 - robbiew
 *	[3] Odd count of read and write
 *	[4] Read beyond the file size
 *	[5] Invalid file descriptor
 *	[6] Out of range file descriptor
 *	[7] Closed file descriptor
 *	[8] Directory read, write - removed 10/7/2002 - plars
 *	[9] Character device (/dev/null) read, write
 *	[10] read, write to a mmaped file
 *	[11] read, write to an unmaped file with munmap
 *	[12] read from file not open for reading
 *	[13] write to file not open for writing
 *	[14] read, write with non-aligned buffer
 *	[15] read, write buffer in read-only space
 *	[16] read, write in non-existant space
 *	[17] read, write for file with O_SYNC
 *
 * USAGE
 *      diotest4 [-b filesize_in_blocks]
 * 
 * History
 *	04/22/2002	Narasimha Sharoff nsharoff@us.ibm.com
 *
 * RESTRICTIONS
 *	None
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#include "diotest_routines.h"

#include "test.h"
#include "usctest.h"

char *TCID="diotest4";		 		 /* Test program identifier.    */
int TST_TOTAL=1;		 		 /* Total number of test conditions */

#ifdef O_DIRECT

#define BUFSIZE 	4096
#define TRUE	1
#define LEN	30

char    *valloc();


/*
 * runtest_f: Do read, writes. Verify the error value obtained by
 *	running read or write with the expected error value (errnum).
*/
int
runtest_f(int fd, char *buf, int   offset, int count, int errnum, int testnum, char *msg)
{
	int ret;
	int l_fail = 0;
	
	if (lseek(fd, offset, SEEK_SET) < 0) {
		if (errno != errnum) {
			fprintf(stderr, "[%d] lseek before read failed: %s\n", 
				testnum, strerror(errno));
			l_fail = TRUE;
		}
	}
	else {
		ret=read(fd, buf, count);
		if (ret >= 0 || errno != errnum) {
			fprintf(stderr, "[%d] read allows %s. returns %d: %s\n",
				testnum, msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		if (errno != errnum) {
			fprintf(stderr, "[%d] lseek before write failed: %s\n", 
				testnum, strerror(errno));
			l_fail = TRUE;
		}
	}
	else {
		ret=write(fd, buf, count);
		if (ret >= 0 || errno != errnum) {
			fprintf(stderr, "[%d] write allows %s.returns %d: %s\n",
				testnum, msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	return(l_fail);
}

/*
 * runtest_s: Do read, writes. Verify the they run successfully.
*/
int
runtest_s(int fd, char *buf, int   offset, int count, int testnum, char *msg)
{
	int ret;
	int l_fail = 0;
	
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[%d] lseek before read failed: %s\n", 
			testnum, strerror(errno));
		l_fail = TRUE;
	}
	else {
		if ((ret=read(fd, buf, count)) < 0) {
			fprintf(stderr, "[%d] read failed for %s. returns %d: %s\n",
				testnum, msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[%d] lseek before write failed: %s\n", 
			testnum, strerror(errno));
		l_fail = TRUE;
	}
	else {
		if ((ret=write(fd, buf, count)) < 0) {
			fprintf(stderr, "[%d] write failed for %s. returns %d: %s\n",
				testnum, msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	return(l_fail);
}

/*
 * prg_usage - Display the program usage
*/
void
prg_usage()
{
        fprintf(stderr, "Usage: diotest4 [-b filesize_in_blocks]\n");
        exit(1);
}

int
main(int argc, char *argv[])
{
	int	fblocks = 1;		/* Iterations. Default 1 */
        int     bufsize = BUFSIZE;
        int     count, ret; 
	int     offset;
        int     fd, newfd, fd1;
        int     i, l_fail = 0, fail_count = 0, total = 0;
	int	failed = 0;
     	int	pgsz = getpagesize();
	int	pagemask = ~(sysconf(_SC_PAGE_SIZE) - 1);
        char    *buf0, *buf1, *buf2;
	char	filename[LEN]; 
	caddr_t	shm_base;
	struct	sigaction act;

	/* Options */
	while ((i = getopt(argc, argv, "b:")) != -1) {
		switch(i) {
		case 'b':
			if ((fblocks = atoi(optarg)) <= 0) {
				fprintf(stderr, "fblocks must be > 0\n");
				prg_usage();
			}
			break;
		default:
			prg_usage();
		}
	}
	act.sa_handler = SIG_IGN;
	(void) sigaction(SIGXFSZ, &act, NULL);
        sprintf(filename,"testdata-4.%d", getpid()); 

        /* Test for filesystem support of O_DIRECT */
        if ((fd1 = open(filename, O_DIRECT|O_CREAT, 0666)) < 0) {
                 tst_resm(TCONF,"O_DIRECT is not supported by this filesystem.");
                 tst_exit();
        }else{
                close(fd1);
        }

	/* Open file and fill, allocate for buffer */
        if ((fd = open(filename, O_DIRECT|O_RDWR|O_CREAT, 0666)) < 0) {
		fprintf(stderr, "open failed for %s: %s\n", 
			filename, strerror(errno));
                exit(1);
        }
        if ((buf0 = valloc(BUFSIZE)) == NULL) {
		fprintf(stderr, "valloc buf0 failed:%s\n", strerror(errno));
                unlink(filename);
                exit(1);
        }
        for (i = 1; i < fblocks; i++) {
                fillbuf(buf0, BUFSIZE, (char)i);
                if (write(fd, buf0, BUFSIZE) < 0) {
			fprintf(stderr, "write failed for %s:%s\n",
				filename, strerror(errno));
                        unlink(filename);
                        exit(1);
                }
        }
	close(fd);
        if ((buf2 = valloc(BUFSIZE)) == NULL) {
		fprintf(stderr, "valloc buf2 failed:%s\n", strerror(errno));
                unlink(filename);
                exit(1);
        }
        if ((fd = open(filename, O_DIRECT|O_RDWR)) < 0) {
		fprintf(stderr, "open failed for %s:%s\n",
				filename, strerror(errno));
                unlink(filename);
                exit(1);
        }

	/* Test-1: Negative Offset */
	offset = -1;
	count = bufsize;
	ret = lseek(fd, offset, SEEK_SET);
	if ((ret >= 0) || (errno != EINVAL)) {
		fprintf(stderr, "[1] lseek allows negative offset. returns %d:%s",
			ret, strerror(errno));
		failed = TRUE;
		fail_count++;
	}
	total++;

	/* Test-3: Odd count of read and write */
	offset = 0;
	count = 1;
	lseek(fd, 0, SEEK_SET);
	if (write(fd, buf2, 4096) == -1) { 
		fprintf(stderr,"[3] can't write to file %d\n",ret);
	}
	ret = runtest_f(fd, buf2, offset, count, EINVAL, 3, "odd count");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;

	/* Test-4: Read beyond the file size */
	offset = BUFSIZE * (fblocks + 10);
	count = bufsize;
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[4.1] lseek failed: %s\n", strerror(errno));
		failed = TRUE;
		fail_count++;
	}
	else {
        	ret = read(fd, buf2, count);
        	if (ret > 0 || errno != EINVAL) {   
                	fprintf(stderr,"[4] allows read beyond file size. returns %d:%s\n",
				ret, strerror(errno));
                	failed = TRUE;
			fail_count++;
		}
	}
	total++;

	/* Test-5: Invalid file descriptor */
	offset = 4096;
	count = bufsize;
	newfd = -1;
	ret = runtest_f(newfd, buf2, offset, count, EBADF, 5, "negative fd");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;

	/* Test-6: Out of range file descriptor */
	count = bufsize;
	offset = 4096;
	if ((newfd = getdtablesize()) < 0) {
		fprintf(stderr, "[6] getdtablesize failed:%s\n", strerror(errno));
                failed = TRUE;
	}
	ret = runtest_f(newfd, buf2, offset, count, EBADF, 6,"out of range fd");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;


	/* Test-7: Closed file descriptor */
	offset = 4096;
	count = bufsize;
        if (close(fd) < 0) {
		fprintf(stderr, "can't close fd %d: %s\n", fd, strerror(errno));
                unlink(filename);
                exit(1);
        }
	ret = runtest_f(fd, buf2, offset, count, EBADF, 7, "closed fd");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;

	/* Test-9: Character device (/dev/null) read, write */
	offset = 0;
	count = bufsize;
        if ((newfd = open("/dev/null", O_DIRECT|O_RDWR)) < 0) {
		fprintf(stderr, "[9] Direct I/O on /dev/null is not supported, skip test #9.\n");
        } else { 
		ret = runtest_s(newfd, buf2, offset, count, 9, "/dev/null");
		if (ret != 0) {
			failed = TRUE;
			fail_count++;
		}
	}
	total++;
	

	/* Test-10: read, write to a mmaped file */
  	shm_base = (char *)(((long)sbrk(0) + (pgsz-1)) & ~(pgsz-1));
        if (shm_base == NULL) {
		fprintf(stderr, "[10] sbrk failed:%s\n", strerror(errno));
                unlink(filename);
                exit(1);
        }
	offset = 4096;
	count = bufsize;
	if ((fd = open(filename, O_DIRECT|O_RDWR)) < 0) {
		fprintf(stderr, "can't open %s: %s\n", 
			filename, strerror(errno));
                unlink(filename);
                exit(1);
        }
	shm_base = mmap(shm_base, 0x100000, PROT_READ|PROT_WRITE, 
		        MAP_SHARED|MAP_FIXED, fd, 0);
        if (shm_base == (caddr_t)-1) {
                fprintf(stderr, "[10] can't mmap file:%s\n", strerror(errno));
                unlink(filename);
                exit(1);
        }
	ret = runtest_s(fd, buf2, offset, count, 10, "mmapped file");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;  


	/* Test-11: read, write to an unmaped file with munmap */
	if ((ret = munmap(shm_base, 0x100000)) < 0) {
                fprintf(stderr, "[11] can't unmap file:%s\n", strerror(errno));
                unlink(filename);
                exit(1);
        }
	ret = runtest_s(fd, buf2, offset, count, 11, "unmapped file");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;


	/* Test-12: read from file not open for reading */
	offset = 4096;
	count = bufsize;
        if ((fd = open(filename, O_DIRECT|O_WRONLY)) < 0) {
		fprintf(stderr, "[12] can't open %s: %s\n",
			filename, strerror(errno));
                unlink(filename);
                exit(1);
        }
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[12] lseek failed:%s\n", strerror(errno));
		failed = TRUE;
		fail_count++;
	}
	else {
		ret = read(fd, buf2, count);
		if (ret >= 0 || errno != EBADF) {
			fprintf(stderr, "[12] allows read on file not open for reading. returns %d: %s\n",
				ret, strerror(errno));
			failed = TRUE;
			fail_count++;
		}
	}
	close(fd);
	total++;

	/* Test-13: write to file not open for writing */
	offset = 4096;
	count = bufsize;
	if ((fd = open(filename, O_DIRECT|O_RDONLY)) < 0) {
		fprintf(stderr, "[13] can't open %s: %s\n",
			filename, strerror(errno));
		unlink(filename);
		exit(1);
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[13] lseek failed:%s\n", strerror(errno));
		failed = TRUE;
		fail_count++;
	}
	else {
		ret = write(fd, buf2, count);
        	if (ret >= 0 || errno != EBADF) {
			fprintf(stderr, "[13] allows write on file not open for writing. returns %d: %s\n",
				ret, strerror(errno));
			failed = TRUE;
			fail_count++;
		}
	}
	close(fd);
	total++;

	/* Test-14: read, write with non-aligned buffer */
	offset = 4096;
	count = bufsize;
	l_fail = 0;
        if ((fd = open(filename, O_DIRECT|O_RDWR)) < 0) {
		fprintf(stderr, "[14] can't open %s: %s\n",
			filename, strerror(errno));
                unlink(filename);
                exit(1);
        }
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[14] lseek before read failed: %s\n",
			strerror(errno));
		l_fail = TRUE;
	}
	else {
		if ((ret = read(fd, buf2+1, count)) != -1) {
                	fprintf(stderr,"[14] allows read nonaligned buf. returns %d:%s\n",
				ret, strerror(errno));
                	l_fail = TRUE;
        	}
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[14] lseek before read failed: %s\n",
			strerror(errno));
		l_fail = TRUE;
	}
	else {
		if ((ret = write(fd, buf2+1, count)) != -1) {
			fprintf(stderr,"[14] allows write nonaligned buf. returns %d:%s\n",
				ret, strerror(errno));
                	l_fail = TRUE;
        	}
	}
	if (l_fail) {
		failed = TRUE;
		fail_count++;
	}
	total++;
	close(fd);


	/* Test-15: read, write buffer in read-only space */
	offset = 4096;
	count = bufsize;
	l_fail = 0;
        if ((fd = open(filename, O_DIRECT|O_RDWR)) < 0) {
                fprintf(stderr, "[15] can't open %s: %s\n",
			filename, strerror(errno));
                unlink(filename);
                exit(1);
        }
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[15] lseek before read failed: %s\n",
			strerror(errno));
		l_fail = TRUE;
	}
	else {
#if defined(__powerpc64__)
		ret = read(fd, (char*)(((ulong *)main)[0] & pagemask), count);
#else
		ret = read(fd, (char*)((ulong)main & pagemask), count);
#endif
		if (ret >= 0 || errno != EFAULT) {   
			fprintf(stderr,"[15] read to read-only space. returns %d:%s\n",
				ret, strerror(errno));
			l_fail = TRUE;
        	}
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		fprintf(stderr, "[15] lseek before write failed: %s\n",
			strerror(errno));
		l_fail = TRUE;
	}
	else {
#if defined(__powerpc64__)
		ret = write(fd, (char *)(((ulong *)main)[0] & pagemask), count);
#else
		ret = write(fd, (char *)((ulong)main & pagemask), count);
#endif
		if (ret < 0 ) {
			fprintf(stderr,"[15] write to read-only space. returns %d:%s\n",
				ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	if (l_fail) {
		failed = TRUE;
		fail_count++;
	}
	close(fd);
	total++;

	/* Test-16: read, write in non-existant space */
	offset = 4096;
	count = bufsize;
	if ((buf1 = (char *) (((long)sbrk(0) + (pgsz-1)) & ~(pgsz-1))) == NULL) {
                fprintf(stderr,"[20] sbrk:%s\n", strerror(errno));
                unlink(filename);
                exit(1);
        }
        if ((fd = open(filename, O_DIRECT|O_RDWR)) < 0) {
		fprintf(stderr, "[16] can't open %s: %s\n",
			filename, strerror(errno));
                unlink(filename);
                exit(1);
        }
	ret =runtest_f(fd, buf1, offset, count, EFAULT, 16, " nonexistant space");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;
	close(fd);

	/* Test-17: read, write for file with O_SYNC */
	offset = 4096;
	count = bufsize;
        if ((fd = open(filename,O_DIRECT|O_RDWR|O_SYNC)) < 0) {
		fprintf(stderr, "[17] can't open %s:%s\n",
			filename, strerror(errno));
                unlink(filename);
                exit(1);
        }
	ret = runtest_s(fd, buf2, offset, count, 17, "opened with O_SYNC");
	if (ret != 0) {
		failed = TRUE;
		fail_count++;
	}
	total++;
	close(fd);
        unlink(filename);
	if (failed) {
		fprintf(stderr, "diotest4: %d/%d test blocks failed\n",
			fail_count, total);
		exit(1);
	}
	fprintf(stderr, "diotest4: %d testblocks completed\n",
		 		 total);
	exit(0);
}

#else /* O_DIRECT */

int
main() {

		 tst_resm(TCONF,"O_DIRECT is not defined.");
		 return 0;
}
#endif /* O_DIRECT */
