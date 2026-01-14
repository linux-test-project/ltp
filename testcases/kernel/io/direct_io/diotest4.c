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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <errno.h>

#include "diotest_routines.h"

#include "test.h"
#include "tso_safe_macros.h"
#include "lapi/mmap.h"

char *TCID = "diotest4";	/* Test program identifier.    */
int TST_TOTAL = 17;		/* Total number of test conditions */

static long fs_type;

#ifdef O_DIRECT

#define BUFSIZE 	4096
#define TRUE	1
#define LEN	30

#ifdef __GNUC__
#define ADDRESS_OF_MAIN __builtin_extract_return_addr(__builtin_return_address(0))
#else
#define ADDRESS_OF_MAIN main
#endif

/*
 * runtest_f: Do read, writes. Verify the error value obtained by
 *	running read or write with the expected error value (errnum).
*/
int
runtest_f(int fd, char *buf, int offset, int count, int errnum, int testnum,
	  char *msg)
{
	int ret;
	int l_fail = 0;

	if (lseek(fd, offset, SEEK_SET) < 0) {
		if (errno != errnum) {
			tst_resm(TFAIL, "lseek before read failed: %s",
				 strerror(errno));
			l_fail = TRUE;
		}
	} else {
		errno = 0;
		ret = read(fd, buf, count);
		if (ret >= 0 || errno != errnum) {
			tst_resm(TFAIL, "read allows %s. returns %d: %s",
				 msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		if (errno != errnum) {
			tst_resm(TFAIL, "lseek before write failed: %s",
				 strerror(errno));
			l_fail = TRUE;
		}
	} else {
		errno = 0;
		ret = write(fd, buf, count);
		if (ret >= 0 || errno != errnum) {
			tst_resm(TFAIL, "write allows %s.returns %d: %s",
				 msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	return (l_fail);
}

/*
 * runtest_s: Do read, writes. Verify the they run successfully.
*/
int runtest_s(int fd, char *buf, int offset, int count, int testnum, char *msg)
{
	int ret;
	int l_fail = 0;

	if (lseek(fd, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek before read failed: %s",
			 strerror(errno));
		l_fail = TRUE;
	} else {
		if ((ret = read(fd, buf, count)) < 0) {
			tst_resm(TFAIL, "read failed for %s. returns %d: %s",
				 msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek before write failed: %s",
			 strerror(errno));
		l_fail = TRUE;
	} else {
		if ((ret = write(fd, buf, count)) < 0) {
			tst_resm(TFAIL, "write failed for %s. returns %d: %s",
				 msg, ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	return (l_fail);
}

static void prg_usage(void)
{
	fprintf(stderr, "Usage: diotest4 [-b filesize_in_blocks]\n");
	exit(1);
}

static void testcheck_end(int ret, int *failed, int *fail_count, char *msg)
{
	if (ret != 0) {
		*failed = TRUE;
		(*fail_count)++;
		tst_resm(TFAIL, "%s", msg);
	} else
		tst_resm(TPASS, "%s", msg);
}

static void setup(void);
static void cleanup(void);
static int fd1 = -1;
static char filename[LEN];

int main(int argc, char *argv[])
{
	int fblocks = 1;	/* Iterations. Default 1 */
	int bufsize = BUFSIZE;
	int count, ret;
	int offset;
	int fd, newfd;
	int i, l_fail = 0, fail_count = 0, total = 0;
	int failed = 0;
	int shmsz = MMAP_GRANULARITY;
	int pagemask = ~(sysconf(_SC_PAGE_SIZE) - 1);
	char *buf0, *buf1, *buf2;
	caddr_t shm_base;

	/* Options */
	while ((i = getopt(argc, argv, "b:")) != -1) {
		switch (i) {
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

	setup();

	/* Open file and fill, allocate for buffer */
	if ((fd = open(filename, O_DIRECT | O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed for %s: %s",
			 filename, strerror(errno));
	}
	if ((buf0 = valloc(bufsize)) == NULL) {
		tst_brkm(TBROK, cleanup, "valloc() buf0 failed: %s",
			 strerror(errno));
	}
	for (i = 1; i < fblocks; i++) {
		fillbuf(buf0, bufsize, (char)i);
		if (write(fd, buf0, bufsize) < 0) {
			tst_brkm(TBROK, cleanup, "write failed for %s: %s",
				 filename, strerror(errno));
		}
	}
	close(fd);
	if ((buf2 = valloc(bufsize)) == NULL) {
		tst_brkm(TBROK, cleanup, "valloc() buf2 failed: %s",
			 strerror(errno));
	}
	if ((fd = open(filename, O_DIRECT | O_RDWR)) < 0) {
		tst_brkm(TBROK, cleanup, "open failed for %s: %s",
			 filename, strerror(errno));
	}

	/* Test-1: Negative Offset */
	offset = -1;
	count = bufsize;
	errno = 0;
	ret = lseek(fd, offset, SEEK_SET);
	if ((ret >= 0) || (errno != EINVAL)) {
		tst_resm(TFAIL, "lseek allows negative offset. returns %d:%s",
			 ret, strerror(errno));
		failed = TRUE;
		fail_count++;
	} else
		tst_resm(TPASS, "Negative Offset");
	total++;

	/* Test-2: Removed */
	tst_resm(TPASS, "removed");

	/* Test-3: Odd count of read and write */
	offset = 0;
	count = 1;
	lseek(fd, 0, SEEK_SET);
	if (write(fd, buf2, 4096) == -1) {
		tst_resm(TFAIL, "can't write to file %d", ret);
	}
	switch (fs_type) {
	case TST_NFS_MAGIC:
	case TST_BTRFS_MAGIC:
	case TST_FUSE_MAGIC:
	case TST_TMPFS_MAGIC:
		tst_resm(TCONF, "%s supports odd count IO",
			 tst_fs_type_name(fs_type));
	break;
	default:
		ret = runtest_f(fd, buf2, offset, count, EINVAL, 3, "odd count");
		testcheck_end(ret, &failed, &fail_count,
					"Odd count of read and write");
	}

	total++;

	/* Test-4: Read beyond the file size */
	offset = bufsize * (fblocks + 10);
	count = bufsize;
	if (lseek(fd, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek failed: %s", strerror(errno));
		failed = TRUE;
		fail_count++;
		tst_resm(TFAIL, "Read beyond the file size");
	} else {
		errno = 0;
		ret = read(fd, buf2, count);
		if (ret > 0 || (ret < 0 && errno != EINVAL)) {
			tst_resm(TFAIL,
				 "allows read beyond file size. returns %d: %s",
				 ret, strerror(errno));
			failed = TRUE;
			fail_count++;
		} else
			tst_resm(TPASS, "Read beyond the file size");
	}
	total++;

	/* Test-5: Invalid file descriptor */
	offset = 4096;
	count = bufsize;
	newfd = -1;
	ret = runtest_f(newfd, buf2, offset, count, EBADF, 5, "negative fd");
	testcheck_end(ret, &failed, &fail_count, "Invalid file descriptor");
	total++;

	/* Test-6: Out of range file descriptor */
	count = bufsize;
	offset = 4096;
	if ((newfd = getdtablesize()) < 0) {
		tst_resm(TFAIL, "getdtablesize() failed: %s", strerror(errno));
		failed = TRUE;
		tst_resm(TFAIL, "Out of range file descriptor");
	} else {
		ret = runtest_f(newfd, buf2, offset, count, EBADF, 6,
			      "out of range fd");
		testcheck_end(ret, &failed, &fail_count,
					"Out of range file descriptor");
	}
	close(newfd);
	total++;

	/* Test-7: Closed file descriptor */
	offset = 4096;
	count = bufsize;
	SAFE_CLOSE(cleanup, fd);
	ret = runtest_f(fd, buf2, offset, count, EBADF, 7, "closed fd");
	testcheck_end(ret, &failed, &fail_count, "Closed file descriptor");
	total++;

	/* Test-9: removed */
	tst_resm(TPASS, "removed");

	/* Test-9: Character device (/dev/null) read, write */
	offset = 0;
	count = bufsize;
	if ((newfd = open("/dev/null", O_DIRECT | O_RDWR)) < 0) {
		tst_resm(TCONF, "Direct I/O on /dev/null is not supported");
	} else {
		ret = runtest_s(newfd, buf2, offset, count, 9, "/dev/null");
		testcheck_end(ret, &failed, &fail_count,
					"character device read, write");
	}
	close(newfd);
	total++;

	/* Test-10: read, write to a mmaped file */
	offset = 4096;
	count = bufsize;
	if ((fd = open(filename, O_DIRECT | O_RDWR)) < 0) {
		tst_brkm(TBROK, cleanup, "can't open %s: %s",
			 filename, strerror(errno));
	}
	shm_base = mmap(0, 0x100000, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, 0);
	if (shm_base == (caddr_t) - 1) {
		tst_brkm(TBROK, cleanup, "can't mmap file: %s",
			 strerror(errno));
	}
	ret = runtest_s(fd, buf2, offset, count, 10, "mmapped file");
	testcheck_end(ret, &failed, &fail_count,
				"read, write to a mmaped file");
	total++;

	/* Test-11: read, write to an unmaped file with munmap */
	if ((ret = munmap(shm_base, 0x100000)) < 0) {
		tst_brkm(TBROK, cleanup, "can't unmap file: %s",
			 strerror(errno));
	}
	ret = runtest_s(fd, buf2, offset, count, 11, "unmapped file");
	testcheck_end(ret, &failed, &fail_count,
				"read, write to an unmapped file");
	close(fd);
	total++;

	/* Test-12: read from file not open for reading */
	offset = 4096;
	count = bufsize;
	if ((fd = open(filename, O_DIRECT | O_WRONLY)) < 0) {
		tst_brkm(TBROK, cleanup, "can't open %s: %s",
			 filename, strerror(errno));
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek failed: %s", strerror(errno));
		failed = TRUE;
		fail_count++;
	} else {
		errno = 0;
		ret = read(fd, buf2, count);
		if (ret >= 0 || errno != EBADF) {
			tst_resm(TFAIL,
				 "allows read on file not open for reading. returns %d: %s",
				 ret, strerror(errno));
			failed = TRUE;
			fail_count++;
		} else
			tst_resm(TPASS, "read from file not open for reading");
	}
	close(fd);
	total++;

	/* Test-13: write to file not open for writing */
	offset = 4096;
	count = bufsize;
	if ((fd = open(filename, O_DIRECT | O_RDONLY)) < 0) {
		tst_brkm(TBROK, cleanup, "can't open %s: %s",
			 filename, strerror(errno));
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek failed: %s", strerror(errno));
		failed = TRUE;
		fail_count++;
	} else {
		errno = 0;
		ret = write(fd, buf2, count);
		if (ret >= 0 || errno != EBADF) {
			tst_resm(TFAIL,
				 "allows write on file not open for writing. returns %d: %s",
				 ret, strerror(errno));
			failed = TRUE;
			fail_count++;
		} else
			tst_resm(TPASS, "write to file not open for writing");
	}
	close(fd);
	total++;

	/* Test-14: read, write with non-aligned buffer */
	offset = 4096;
	count = bufsize;
	if ((fd = open(filename, O_DIRECT | O_RDWR)) < 0) {
		tst_brkm(TBROK, cleanup, "can't open %s: %s",
			 filename, strerror(errno));
	}
	switch (fs_type) {
	case TST_NFS_MAGIC:
	case TST_BTRFS_MAGIC:
	case TST_FUSE_MAGIC:
	case TST_TMPFS_MAGIC:
		tst_resm(TCONF, "%s supports non-aligned buffer",
			 tst_fs_type_name(fs_type));
	break;
	default:
		ret = runtest_f(fd, buf2 + 1, offset, count, EINVAL, 14,
					" nonaligned buf");
		testcheck_end(ret, &failed, &fail_count,
				"read, write with non-aligned buffer");
	}
	close(fd);
	total++;

	/* Test-15: read, write buffer in read-only space */
	offset = 4096;
	count = bufsize;
	l_fail = 0;
	if ((fd = open(filename, O_DIRECT | O_RDWR)) < 0) {
		tst_brkm(TBROK, cleanup, "can't open %s: %s",
			 filename, strerror(errno));
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek before read failed: %s",
			 strerror(errno));
		l_fail = TRUE;
	} else {
		errno = 0;
		ret = read(fd, (char *)((ulong) ADDRESS_OF_MAIN & pagemask),
			 count);
		if (ret >= 0 || errno != EFAULT) {
			tst_resm(TFAIL,
				 "read to read-only space. returns %d: %s", ret,
				 strerror(errno));
			l_fail = TRUE;
		}
	}
	if (lseek(fd, offset, SEEK_SET) < 0) {
		tst_resm(TFAIL, "lseek before write failed: %s",
			 strerror(errno));
		l_fail = TRUE;
	} else {
		ret = write(fd, (char *)((ulong) ADDRESS_OF_MAIN & pagemask),
			  count);
		if (ret < 0) {
			tst_resm(TFAIL,
				 "write to read-only space. returns %d: %s",
				 ret, strerror(errno));
			l_fail = TRUE;
		}
	}
	testcheck_end(l_fail, &failed, &fail_count,
				"read, write buffer in read-only space");
	close(fd);
	total++;

	/* Test-16: read, write in non-existant space */
	offset = 4096;
	count = bufsize;
	if ((buf1 =
	     (char *)(((long)sbrk(0) + (shmsz - 1)) & ~(shmsz - 1))) == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup, "sbrk failed");
	}
	if ((fd = open(filename, O_DIRECT | O_RDWR)) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_DIRECT|O_RDWR) failed", filename);
	}
	ret = runtest_f(fd, buf1, offset, count, EFAULT, 16,
		      " nonexistant space");
	testcheck_end(ret, &failed, &fail_count,
				"read, write in non-existant space");
	total++;
	close(fd);

	/* Test-17: read, write for file with O_SYNC */
	offset = 4096;
	count = bufsize;
	if ((fd = open(filename, O_DIRECT | O_RDWR | O_SYNC)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_DIRECT|O_RDWR|O_SYNC failed)", filename);
	}
	ret = runtest_s(fd, buf2, offset, count, 17, "opened with O_SYNC");
	testcheck_end(ret, &failed, &fail_count,
				"read, write for file with O_SYNC");
	total++;
	close(fd);

	unlink(filename);
	if (failed)
		tst_resm(TINFO, "%d/%d test blocks failed", fail_count, total);
	else
		tst_resm(TINFO, "%d testblocks completed", total);
	cleanup();

	tst_exit();
}

static void setup(void)
{
	struct sigaction act;

	tst_tmpdir();

	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	(void)sigaction(SIGXFSZ, &act, NULL);
	sprintf(filename, "testdata-4.%ld", syscall(__NR_gettid));

	if ((fd1 = open(filename, O_CREAT | O_EXCL, 0600)) < 0) {
		tst_brkm(TBROK, cleanup, "Couldn't create test file %s: %s",
			 filename, strerror(errno));
	}
	close(fd1);

	/* Test for filesystem support of O_DIRECT */
	if ((fd1 = open(filename, O_DIRECT, 0600)) < 0) {
		tst_brkm(TCONF, cleanup,
			 "O_DIRECT is not supported by this filesystem. %s",
			 strerror(errno));
	}
	close(fd1);

	fs_type = tst_fs_type(cleanup, ".");
}

static void cleanup(void)
{
	if (fd1 != -1)
		unlink(filename);

	tst_rmdir();

}

#else /* O_DIRECT */

int main()
{
	tst_brkm(TCONF, NULL, "O_DIRECT is not defined.");
}
#endif /* O_DIRECT */
