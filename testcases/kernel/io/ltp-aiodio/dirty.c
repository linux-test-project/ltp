
/*
 * Copyright (c) 2004 Daniel McNeil <daniel@osdl.org>
 *               2004 Open Source Development Lab
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
 *
 * Module: .c
 */

/*
 * Change History:
 *
 * 2/2004  Marty Ridgeway (mridge@us.ibm.com) Changes to adapt to LTP
 *
 */

#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int main(void)
{
	int fd;
	int i;
	char buf[32 * 1024];
	char filename[PATH_MAX];

	printf("Starting dirty tests...\n");

	snprintf(filename, sizeof(filename), "%s/aiodio/file.xx.%d",
		 getenv("TMP") ? getenv("TMP") : "/tmp", getpid());

	fd = open(filename, O_CREAT | O_WRONLY, 0666);

	memset(buf, 0xaa, sizeof(buf));
	for (i = 0; i < 3000; i++)
		write(fd, buf, sizeof(buf));
	fsync(fd);
	close(fd);
	unlink(filename);
	return 0;
}
