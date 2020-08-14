/*
 *The <errno.h> header shall provide a
 *declaration for errno
 *author:ysun@lnxw.com
 */

#include <errno.h>

static int errno_test;

static int dummyfcn(void)
{
	errno_test = errno;
	return 0;
}
