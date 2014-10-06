/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "test.h"

char *TCID = "setpgid03_child";


int main(void)
{
	struct tst_checkpoint checkpoint;

	/* we are already in tmpdir, so only initialize checkpoint,
	 * fifo has been created by parent already. */
	TST_CHECKPOINT_INIT(&checkpoint);
	checkpoint.timeout = 10000;

	TST_CHECKPOINT_SIGNAL_PARENT(&checkpoint);
	TST_CHECKPOINT_CHILD_WAIT(&checkpoint);

	return 0;
}
