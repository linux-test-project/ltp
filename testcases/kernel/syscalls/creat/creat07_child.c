/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (c) 2012 Cyril Hrubis <chrubis@suse.cz>
 *
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

#include <stdio.h>
#include <unistd.h>

#include "test.h"

char *TCID = "creat07_child";

static struct tst_checkpoint checkpoint = {.timeout = 5, .retval = 1};

int main(void)
{
	TST_CHECKPOINT_SIGNAL_PARENT(&checkpoint);

	for (;;) {
		sleep(1);
	}
	return 0;
}
