#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "test.h"

#ifdef HAVE_LIBAIO_H

#include <libaio.h>

char test_name[] = TEST_NAME;

#include TEST_NAME

int main(void)
{
	int res;

	res = test_main();
	printf("test %s completed %s.\n", test_name, 
		res ? "FAILED" : "PASSED"
		);
	fflush(stdout);
	return res ? 1 : 0;
}
#else
char *TCID = "aio02/" TEST_NAME;
int TST_TOTAL=0;

int main(void)
{
  tst_brkm(TCONF, tst_exit, "libaio missing");
}

#endif
