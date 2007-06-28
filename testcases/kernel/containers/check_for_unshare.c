#include <stdio.h>
#include <test.h>

int kernel_is_too_old(void) {
	if (tst_kvercmp(2,6,16) < 0)
		return 1;
	return 0;
}

/*
 * yeah, to make the makefile coding easier, do_check returns 
 * 1 if unshare is not supported, 0 if it is
 */
#ifndef SYS_unshare
#ifdef __NR_unshare
int do_check(void) { return kernel_is_too_old(); }
#elif __i386__
int do_check(void) { return kernel_is_too_old(); }
#elif __ia64__
int do_check(void) { return kernel_is_too_old(); }
#elif __x86_64__
int do_check(void) { return kernel_is_too_old(); }
#elif __s390x__ || __s390__
int do_check(void) { return kernel_is_too_old(); }
#elif __powerpc__
int do_check(void) { return kernel_is_too_old(); }
#else
int do_check(void) { return 1; }
#endif
#endif

int main() {
	return do_check();
}
