/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * Test that the shm_open() function sets errno = EINVAL if the shm_open()
 * operation is not supported for the given name.
 *
 * The supported names are implementation-defined, so the test is done for
 * several differents names. The test pass for a given name if shm_open make no
 * error or set errno to EINVAL.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "posixtest.h"

char *shm_name[] = {
	/* char which are in portable character set but not in portable
	   filename character set */
	"$#\n@\t\a,~}",
	/* char which are not in portable character set (accentuated char and c
	   cedilla) */
	"йкофза",
	/* some file or directory which should exist */
	"..",
	"/",
	"//",
	"/abc",
	NULL
};

int main(void)
{
	int fd, i = 0, result = PTS_PASS;

	while (shm_name[i]) {
		fflush(stderr);
		printf("Name: '%s'\n", shm_name[i]);
		fflush(stdout);

		fd = shm_open(shm_name[i], O_RDWR | O_CREAT, 0);

		if (fd == -1 && errno == EINVAL) {
			printf("   OK: errno == EINVAL\n");
		} else if (fd != -1) {
			printf("   OK: open  with success.\n");
		} else {
			perror("   Unexpected error");
			result = PTS_FAIL;
		}

		shm_unlink(shm_name[i]);

		i++;
	}

	if (result == PTS_PASS)
		printf("Test PASSED\n");
	else
		printf("Test FAILED\n");
	return result;
}
