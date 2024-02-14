// SPDX-License-Identifier: GPL-2.0-only
/*
 * Test that the shm_open() function sets errno = EINVAL if the shm_open()
 * operation is not supported for the given name.
 *
 * The supported names are implementation-defined, so the test is done for
 * several different names. The test pass for a given name if shm_open make no
 * error or set errno to EINVAL.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "posixtest.h"

struct test_data {
	const char *desc;
	const char *name;
};

struct test_data testdata[] = {
	{
		"char which are in portable character set, "
		"but not in portable filename character set",
		"$#\n@\t\a,~}"
	},
	{
		"chars which are not in portable character set "
		 "(accentuated char and c cedilla)",
		"\xe9\xea\xee\xf4\xe7\xe0"
	},
	{ "parent directory", ".." },
	{ "root directory", "/" },
	{ "double slash", "//" },
	{ "non-existent directory", "/abc" }
};

int main(void)
{
	unsigned int i;
	int fd, result = PTS_PASS;

	for (i = 0; i < ARRAY_SIZE(testdata); i++) {
		fflush(stderr);
		printf("Test: %s\n", testdata[i].desc);
		fflush(stdout);

		fd = shm_open(testdata[i].name, O_RDWR | O_CREAT, 0);
		if (fd == -1 && errno == EINVAL) {
			printf("   OK: errno == EINVAL\n");
		} else if (fd != -1) {
			printf("   OK: open with success.\n");
			close(fd);
		} else {
			perror("   Unexpected error");
			result = PTS_FAIL;
		}

		shm_unlink(testdata[i].name);
	}

	if (result == PTS_PASS)
		printf("Test PASSED\n");
	else
		printf("Test FAILED\n");
	return result;
}
