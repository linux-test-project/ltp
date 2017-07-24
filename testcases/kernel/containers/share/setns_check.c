/* Copyright (c) 2015 Fujitsu Ltd.
 * Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 2 the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************
 * Check for setns() availability, should be called before ns_exec.
 *
 */

#include "test.h"
#include "lapi/syscalls.h"

char *TCID = "setns_check";

int main(void)
{
	if (syscall(__NR_setns, -1, 0) == -1 && errno == ENOSYS)
		tst_brkm(TCONF, NULL, "setns is not supported in the kernel");
	else
		return 0;
}
