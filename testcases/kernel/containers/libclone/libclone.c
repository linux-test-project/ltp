#include "libclone.h"

/* Serge: should I be passing in strings for error messages? */

int do_clone_tests(unsigned long clone_flags,
			int(*fn1)(void *arg), void *arg1,
			int(*fn2)(void *arg), void *arg2)
{
	int ret;
	void *childstack, *stack = malloc(getpagesize());

	if (!stack) {
		perror("malloc");
		return -1;
	}

	childstack = stack + getpagesize();

#ifdef __ia64__
	ret = clone2(fn1, childstack, getpagesize(), clone_flags | SIGCHLD, arg1, NULL, NULL, NULL);
#else
	ret = clone(fn1, childstack, clone_flags | SIGCHLD, arg1);
#endif

	if (ret == -1) {
		perror("clone");
		free(stack);
		return -1;
	}
	if (fn2)
		ret = fn2(arg2);
	else
		ret = 0;

	return ret;
}

int do_unshare_tests(unsigned long clone_flags,
			int (*fn1)(void *arg), void *arg1,
			int (*fn2)(void *arg), void *arg2)
{
	int pid, ret = 0;
	int retpipe[2];
	char buf[2];

	if (pipe(retpipe) == -1) {
		perror("pipe");
		return -1;
	}
	pid = fork();
	if (pid == -1) {
		perror("fork");
		close(retpipe[0]);
		close(retpipe[1]);
		return -1;
	}
	if (pid == 0) {
		close(retpipe[0]);
		ret = syscall(SYS_unshare, clone_flags);
		if (ret == -1) {
			write(retpipe[1], "0", 2);
			close(retpipe[1]);
			perror("unshare");
			exit(1);
		} else
			write(retpipe[1], "1", 2);
		close(retpipe[1]);
		ret = fn1(arg1);
		exit(ret);
	} else {
		close(retpipe[1]);
		read(retpipe[0], &buf, 2);
		close(retpipe[0]);
		if (*buf == '0')
			return -1;
		if (fn2)
			ret = fn2(arg2);
	}

	return ret;
}

int do_plain_tests(int (*fn1)(void *arg), void *arg1,
			int (*fn2)(void *arg), void *arg2)
{
	int ret = 0, pid;

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}
	if (pid == 0)
		return fn1(arg1);
	if (fn2)
		ret = fn2(arg2);
	return ret;
}

int do_clone_unshare_test(int use_clone, unsigned long clone_flags,
			int (*fn1)(void *arg), void *arg1)
{
	switch (use_clone) {
	case T_NONE:
		return do_plain_tests(fn1, arg1, NULL, NULL);
	case T_CLONE:
		return do_clone_tests(clone_flags, fn1, arg1, NULL, NULL);
	case T_UNSHARE:
		return do_unshare_tests(clone_flags, fn1, arg1, NULL, NULL);
	default:
		printf("%s: bad use_clone option: %d\n", __FUNCTION__,
							use_clone);
		return -1;
	}
}


/*
 * Run fn1 in a unshared environmnent, and fn2 in the original context
 */
int do_clone_unshare_tests(int use_clone, unsigned long clone_flags,
			int (*fn1)(void *arg), void *arg1,
			int (*fn2)(void *arg), void *arg2)
{
	switch (use_clone) {
	case T_NONE:
		return do_plain_tests(fn1, arg1, fn2, arg2);
	case T_CLONE:
		return do_clone_tests(clone_flags, fn1, arg1, fn2, arg2);
	case T_UNSHARE:
		return do_unshare_tests(clone_flags, fn1, arg1, fn2, arg2);
	default:
		printf("%s: bad use_clone option: %d\n", __FUNCTION__,
							use_clone);
		return -1;
	}
}
