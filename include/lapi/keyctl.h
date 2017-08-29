/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KEYCTL_H__
#define KEYCTL_H__

#include "config.h"
#ifdef HAVE_KEYUTILS_H
# include <keyutils.h>
#else
# ifdef HAVE_LINUX_KEYCTL_H
#  include <linux/keyctl.h>
# endif /* HAVE_LINUX_KEYCTL_H */
# include <stdarg.h>
# include <stdint.h>
# include "lapi/syscalls.h"
typedef int32_t key_serial_t;

static inline key_serial_t add_key(const char *type,
				   const char *description,
				   const void *payload,
				   size_t plen,
				   key_serial_t ringid)
{
	return tst_syscall(__NR_add_key,
		type, description, payload, plen, ringid);
}

static inline key_serial_t request_key(const char *type,
				       const char *description,
				       const char *callout_info,
				       key_serial_t destringid)
{
	return tst_syscall(__NR_request_key,
		type, description, callout_info, destringid);
}

static inline long keyctl(int cmd, ...)
{
	va_list va;
	unsigned long arg2, arg3, arg4, arg5;

	va_start(va, cmd);
	arg2 = va_arg(va, unsigned long);
	arg3 = va_arg(va, unsigned long);
	arg4 = va_arg(va, unsigned long);
	arg5 = va_arg(va, unsigned long);
	va_end(va);

	return tst_syscall(__NR_keyctl, cmd, arg2, arg3, arg4, arg5);
}
#endif /* HAVE_KEYUTILS_H */

#ifndef KEYCTL_GET_KEYRING_ID
# define KEYCTL_GET_KEYRING_ID 0
#endif

#ifndef KEYCTL_SET_REQKEY_KEYRING
# define KEYCTL_SET_REQKEY_KEYRING 14
#endif

#ifndef KEYCTL_JOIN_SESSION_KEYRING
# define KEYCTL_JOIN_SESSION_KEYRING 1
#endif

#ifndef KEYCTL_UPDATE
# define KEYCTL_UPDATE 2
#endif

#ifndef KEYCTL_SETPERM
# define KEYCTL_SETPERM 5
#endif

#ifndef KEYCTL_UNLINK
# define KEYCTL_UNLINK 9
#endif

#ifndef KEY_SPEC_THREAD_KEYRING
# define KEY_SPEC_THREAD_KEYRING -1
#endif

#ifndef KEY_SPEC_SESSION_KEYRING
# define KEY_SPEC_SESSION_KEYRING -3
#endif

#ifndef KEY_REQKEY_DEFL_THREAD_KEYRING
# define KEY_REQKEY_DEFL_THREAD_KEYRING 1
#endif

#endif	/* KEYCTL_H__ */
