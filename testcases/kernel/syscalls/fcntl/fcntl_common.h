#include "lapi/syscalls.h"
#include "lapi/abisize.h"

/*
 * glibc commit:
 *   06ab719d30b0 ("Fix Linux fcntl OFD locks for non-LFS architectures (BZ#20251)")
 * changed behavior of arg parameter for OFD commands. It is no
 * longer passing arg directly to syscall, but expects it to be
 * 'struct flock'.
 *
 * On 64-bit or _FILE_OFFSET_BITS == 64 we can use fcntl() and
 * struct flock64 with any glibc version. struct flock and flock64
 * should be identical.
 *
 * On 32-bit, older glibc would pass arg directly, recent one treats
 * it as 'struct flock' and converts it to 'struct flock64'.
 * So, to support both version, on 32-bit we use fcntl64 syscall
 * directly with struct flock64.
 */
#if defined(TST_ABI64) || _FILE_OFFSET_BITS == 64
static int my_fcntl(int fd, int cmd, void *lck)
{
	return SAFE_FCNTL(fd, cmd, lck);
}
#else
static int my_fcntl(int fd, int cmd, void *lck)
{
	int ret = tst_syscall(__NR_fcntl64, fd, cmd, lck);
	if (ret == -1)
		tst_brk(TBROK|TERRNO, "fcntl64");
	return ret;
}
#endif
