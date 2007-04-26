/*
 * Copyright 2007 IBM
 * Author: Serge Hallyn <serue@us.ibm.com>
 *
 * uts namespaces were introduced around 2.6.19.  Kernels before that,
 * assume they are not enabled.  Kernels after that, check for -EINVAL
 * when trying to use CLONE_NEWUTS.
 */

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
	void *childstack, *stack;
	int pid;

	//if (!kernel_version_newenough())
	if (tst_kvercmp(2,6,19) < 0)
		return 1;
	stack = malloc(getpagesize());
	if (!stack) {
		perror("malloc");
		return 2;
	}

	childstack = stack + getpagesize();

	pid = clone(dummy, childstack, CLONE_NEWUTS, NULL);

	if (pid == -1)
		return 3;
	return 0;
}
