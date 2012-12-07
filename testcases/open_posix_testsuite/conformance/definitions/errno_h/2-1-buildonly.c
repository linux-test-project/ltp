/*
 *The <errno.h> header shall provide a
 *declaration for errno
 *author:ysun@lnxw.com
 */

#include <errno.h>

int errno_test;

int dummyfcn(void)
{
	errno_test = errno;
	return 0;
}
