/*
* Copyright (c) International Business Machines Corp., 2007
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
* the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* Author: Serge Hallyn <serue@us.ibm.com>
*
* uts namespaces were introduced around 2.6.19.  Kernels before that,
* assume they are not enabled.  Kernels after that, check for -EINVAL
* when trying to use CLONE_NEWUTS.
***************************************************************************/

#include <sys/utsname.h>
#include <sched.h>
#include <stdio.h>
#include "../libclone/libclone.h"
#include "test.h"

int dummy(void *v)
{
	return 0;
}

/*
 * Not really expecting anyone to use this on a 2.6.19-rc kernel,
 * else we may get some false positives here.
 */
#if 0
int kernel_version_newenough()
{
	int ret;
	struct utsname buf;
	char *s;
	int maj, min, micro;

	ret = uname(&buf);
	if (ret == -1) {
		perror("uname");
		return 0;
	}
	s = buf.release;
	sscanf(s, "%d.%d.%d", &maj, &min, &micro);
	if (maj < 2)
		return 0;
	if (min < 6)
		return 0;
	if (micro < 19)
		return 0;
	return 1;
}
#endif  /* Library is already provided by LTP*/
int main()
{
	int pid;

	//if (!kernel_version_newenough())
	if (tst_kvercmp(2,6,19) < 0)
		return 1;

	pid = ltp_clone_quick(CLONE_NEWUTS, dummy, NULL);

	if (pid == -1)
		return 3;
	return 0;
}
