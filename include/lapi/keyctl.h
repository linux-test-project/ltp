// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef LAPI_KEYCTL_H__
#define LAPI_KEYCTL_H__

#include "config.h"

#if defined(HAVE_KEYUTILS_H) && defined(HAVE_LIBKEYUTILS)
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

static inline key_serial_t keyctl_join_session_keyring(const char *name) {
	return keyctl(KEYCTL_JOIN_SESSION_KEYRING, name);
}

#endif /* defined(HAVE_KEYUTILS_H) && defined(HAVE_LIBKEYUTILS) */

/* special process keyring shortcut IDs */
#ifndef KEY_SPEC_THREAD_KEYRING
# define KEY_SPEC_THREAD_KEYRING -1
#endif

#ifndef KEY_SPEC_PROCESS_KEYRING
# define KEY_SPEC_PROCESS_KEYRING -2
#endif

#ifndef KEY_SPEC_SESSION_KEYRING
# define KEY_SPEC_SESSION_KEYRING -3
#endif

#ifndef KEY_SPEC_USER_KEYRING
# define KEY_SPEC_USER_KEYRING -4
#endif


#ifndef KEY_SPEC_USER_SESSION_KEYRING
# define KEY_SPEC_USER_SESSION_KEYRING -5
#endif

/* request-key default keyrings */
#ifndef KEY_REQKEY_DEFL_THREAD_KEYRING
# define KEY_REQKEY_DEFL_THREAD_KEYRING 1
#endif

#ifndef KEY_REQKEY_DEFL_SESSION_KEYRING
# define KEY_REQKEY_DEFL_SESSION_KEYRING 3
#endif

#ifndef KEY_REQKEY_DEFL_DEFAULT
# define KEY_REQKEY_DEFL_DEFAULT	0
#endif

/* keyctl commands */
#ifndef KEYCTL_GET_KEYRING_ID
# define KEYCTL_GET_KEYRING_ID 0
#endif

#ifndef KEYCTL_JOIN_SESSION_KEYRING
# define KEYCTL_JOIN_SESSION_KEYRING 1
#endif

#ifndef KEYCTL_UPDATE
# define KEYCTL_UPDATE 2
#endif

#ifndef KEYCTL_REVOKE
# define KEYCTL_REVOKE 3
#endif

#ifndef KEYCTL_SETPERM
# define KEYCTL_SETPERM 5
#endif

#ifndef KEYCTL_CLEAR
# define KEYCTL_CLEAR 7
#endif

#ifndef KEYCTL_UNLINK
# define KEYCTL_UNLINK 9
#endif

#ifndef KEYCTL_READ
# define KEYCTL_READ 11
#endif

#ifndef KEYCTL_SET_REQKEY_KEYRING
# define KEYCTL_SET_REQKEY_KEYRING 14
#endif

#ifndef KEYCTL_SET_TIMEOUT
# define KEYCTL_SET_TIMEOUT 15
#endif

#ifndef KEYCTL_INVALIDATE
# define KEYCTL_INVALIDATE 21
#endif

/* key permissions */
#ifndef KEY_POS_VIEW
# define KEY_POS_VIEW    0x01000000
# define KEY_POS_READ    0x02000000
# define KEY_POS_WRITE   0x04000000
# define KEY_POS_SEARCH  0x08000000
# define KEY_POS_LINK    0x10000000
# define KEY_POS_SETATTR 0x20000000
# define KEY_POS_ALL     0x3f000000

# define KEY_USR_VIEW    0x00010000
# define KEY_USR_READ    0x00020000
# define KEY_USR_WRITE   0x00040000
# define KEY_USR_SEARCH  0x00080000
# define KEY_USR_LINK    0x00100000
# define KEY_USR_SETATTR 0x00200000
# define KEY_USR_ALL     0x003f0000

# define KEY_GRP_VIEW    0x00000100
# define KEY_GRP_READ    0x00000200
# define KEY_GRP_WRITE   0x00000400
# define KEY_GRP_SEARCH  0x00000800
# define KEY_GRP_LINK    0x00001000
# define KEY_GRP_SETATTR 0x00002000
# define KEY_GRP_ALL     0x00003f00

# define KEY_OTH_VIEW    0x00000001
# define KEY_OTH_READ    0x00000002
# define KEY_OTH_WRITE   0x00000004
# define KEY_OTH_SEARCH  0x00000008
# define KEY_OTH_LINK    0x00000010
# define KEY_OTH_SETATTR 0x00000020
# define KEY_OTH_ALL     0x0000003f
#endif /* !KEY_POS_VIEW */

#endif	/* LAPI_KEYCTL_H__ */
