/*
 * llseek.c -- stub calling the llseek system call
 *
 * Copyright (C) 1994 Remy Card.  This file may be redistributed
 * under the terms of the GNU Public License.
 *
 * This file is borrowed from the util-linux-2.10q tarball's implementation
 * of fdisk. It allows seeks to 64 bit offsets, if supported.
 * Changed "ext2_" prefix to "llse".
 */

#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/unistd.h>	/* for __NR_llseek */

#if defined(__GNUC__) || defined(HAS_LONG_LONG)
typedef long long llse_loff_t;
#else
typedef long llse_loff_t;
#endif

extern llse_loff_t llse_llseek(unsigned int, llse_loff_t, unsigned int);

#ifdef __linux__

#if defined(__alpha__) || defined(__ia64__)

#ifdef __NR_lseek
static off_t my_lseek(int fd, off_t off, int whence)
{
	return syscall(__NR_lseek, fd, off, whence);
}
#else /* undefined __NR_lseek */
static off_t my_lseek(int fd, off_t off, int whence)
{
	errno = ENOSYS;
	return -1;
}
#endif /* __NR_lseek */

#define my_llseek my_lseek

#else /* !__alpha__ && !__ia64__ */

static int _llseek(unsigned int, unsigned long,
		   unsigned long, llse_loff_t *, unsigned int);

#ifndef __NR_llseek
/* no __NR_llseek on compilation machine - might give it explicitly */
static int _llseek(unsigned int fd, unsigned long oh,
		   unsigned long ol, llse_loff_t * result, unsigned int origin)
{
	errno = ENOSYS;
	return -1;
}
#else
static int _llseek(unsigned int fd, unsigned long oh,
		   unsigned long ol, llse_loff_t * result, unsigned int origin)
{
	return syscall(__NR_llseek, fd, oh, ol, result, origin);
}
#endif

static llse_loff_t my_llseek(unsigned int fd, llse_loff_t offset,
			     unsigned int origin)
{
	llse_loff_t result;
	int retval;

	retval = _llseek(fd, ((unsigned long long)offset) >> 32,
			 ((unsigned long long)offset) & 0xffffffff,
			 &result, origin);
	return (retval == -1 ? (llse_loff_t) retval : result);
}

#endif /* __alpha__ */

llse_loff_t llse_llseek(unsigned int fd, llse_loff_t offset,
			unsigned int origin)
{
	llse_loff_t result;
	static int do_compat = 0;

	if (!do_compat) {
		result = my_llseek(fd, offset, origin);
		if (!(result == -1 && errno == ENOSYS))
			return result;

		/*
		 * Just in case this code runs on top of an old kernel
		 * which does not support the llseek system call
		 */
		do_compat = 1;
		/*
		 * Now try ordinary lseek.
		 */
	}

	if ((sizeof(off_t) >= sizeof(llse_loff_t)) ||
	    (offset < ((llse_loff_t) 1 << ((sizeof(off_t) * 8) - 1))))
		return lseek(fd, (off_t) offset, origin);

	errno = EINVAL;
	return -1;
}

#else /* !linux */

llse_loff_t llse_llseek(unsigned int fd, llse_loff_t offset,
			unsigned int origin)
{
	if ((sizeof(off_t) < sizeof(llse_loff_t)) &&
	    (offset >= ((llse_loff_t) 1 << ((sizeof(off_t) * 8) - 1)))) {
		errno = EINVAL;
		return -1;
	}
	return lseek(fd, (off_t) offset, origin);
}

#endif /* linux */
