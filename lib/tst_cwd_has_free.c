/*
 *    AUTHOR
 *    	Ricky Ng-Adam <rngadam@yahoo.com>, 2005-01-01
 *
 *    DESCRIPTION
 * 	Check if there is enough blocks to fill number of KiB specified
 * 	If current directory has enough blocks, return 1
 * 	If current directory has NOT enough blocks, return 0
 *
 *
 */
#include <sys/vfs.h>

int
tst_cwd_has_free(int required_kib)
{
	struct statfs sf;
	statfs(".", &sf);

	/* check that we have enough blocks to create swap file */
	return ((float)sf.f_bfree)/(1024/sf.f_bsize) >= required_kib?1:0;
}