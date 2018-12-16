/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Copyright (c) 2012, Cyril Hrubis <chrubis@suse.cz>
 *
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The mmap() function shall fail if:
 * [EOVERFLOW] The file is a regular file and the value of off
 * plus len exceeds the offset maximum established in the open
 * file description associated with fildes.
 *
 * Note: This error condition came to the standard with large
 *       file extension and cannot be triggered without it.
 *
 *       So in order to trigger this we need 32 bit architecture
 *       and largefile support turned on.
 */


/* Turn on large file support, has no effect on 64 bit archs */
#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#include "posixtest.h"

int main(void)
{
	char tmpfname[256];

	void *pa;
	size_t len;
	int fd;
	off_t off = 0;

	/* check for 64 bit arch */
	if (sizeof(void *) == 8) {
		printf("USUPPORTED: Cannot be tested on 64 bit architecture\n");
		return PTS_UNSUPPORTED;
	}

	/* The overflow does not happen when 32bit binary runs on 64bit kernel */
#ifdef __linux__
	struct utsname buf;

	if (!uname(&buf) && strstr(buf.machine, "64")) {
		printf("UNSUPPORTED: Looks like we run on 64bit kernel (%s)\n",
		       buf.machine);
		return PTS_UNSUPPORTED;
	}

#endif /* __linux__ */

	long page_size = sysconf(_SC_PAGE_SIZE);

	snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_31_1_%d", getpid());
	unlink(tmpfname);
	fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf("Error at open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
	unlink(tmpfname);

	/* Set lenght to maximal multiple of page size */
	len = ~((size_t) 0) & (~(page_size - 1));

	/*
	 * Now we need offset that fits into 32 bit
	 * value when divided by page size but is big
	 * enough so that offset + PAGE_ALIGN(len) / page_size
	 * overflows 32 bits.
	 */
	off = ((off_t) ~ ((size_t) 0)) * page_size;
	off &= ~(page_size - 1);

	printf("off: %llx, len: %llx\n", (unsigned long long)off,
	       (unsigned long long)len);

	pa = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, off);
	if (pa == MAP_FAILED && errno == EOVERFLOW) {
		printf("Got EOVERFLOW\n");
		printf("Test PASSED\n");
		return PTS_PASS;
	}

	if (pa == MAP_FAILED)
		perror("Test FAILED: expect EOVERFLOW but get other error");
	else
		printf("Test FAILED: Expect EOVERFLOW but got no error\n");

	close(fd);
	munmap(pa, len);
	return PTS_FAIL;
}
