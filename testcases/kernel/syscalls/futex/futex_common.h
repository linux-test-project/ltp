/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * Licensed under the GNU GPLv2 or later.
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

 /*
  * Setups futex in shared memory needed for synchronization between multiple
  * processes.
  */

static futex_t *futex;

static void setup(void)
{
	int fd;

	fd = shm_open("/LTP_futex_wait", O_RDWR | O_CREAT | O_EXCL, 0);

	if (fd < 0) {
		tst_brkm(TBROK | TERRNO, NULL,
		         "shm_open(/LTP_futex_wait,O_RDWR|O_CREAT|O_EXCL,775)");
	}
	if (shm_unlink("/LTP_futex_wait"))
		tst_brkm(TBROK | TERRNO, NULL, "shm_unlink(/LTP_futex_wait)");

	futex = SAFE_MMAP(NULL, NULL, sizeof(*futex), PROT_READ | PROT_WRITE,
			  MAP_ANONYMOUS | MAP_SHARED, fd, 0);

	SAFE_CLOSE(NULL, fd);

	*futex = FUTEX_INITIALIZER;
}
