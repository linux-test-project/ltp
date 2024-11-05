/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef LAPI_LSM_H__
#define LAPI_LSM_H__

#include "config.h"

#ifdef HAVE_LINUX_LSM_H
#include <linux/lsm.h>
#endif

#include <stdint.h>
#include "lapi/syscalls.h"

#define CTX_DATA_SIZE 4096

#define LSM_CTX_SIZE(x) (sizeof(struct lsm_ctx) + x)
#define LSM_CTX_SIZE_DEFAULT LSM_CTX_SIZE(CTX_DATA_SIZE)

#ifndef HAVE_STRUCT_LSM_CTX

/**
 * struct lsm_ctx - LSM context information
 * @id: the LSM id number, see LSM_ID_XXX
 * @flags: LSM specific flags
 * @len: length of the lsm_ctx struct, @ctx and any other data or padding
 * @ctx_len: the size of @ctx
 * @ctx: the LSM context value
 *
 * The @len field MUST be equal to the size of the lsm_ctx struct
 * plus any additional padding and/or data placed after @ctx.
 *
 * In all cases @ctx_len MUST be equal to the length of @ctx.
 * If @ctx is a string value it should be nul terminated with
 * @ctx_len equal to `strlen(@ctx) + 1`.  Binary values are
 * supported.
 *
 * The @flags and @ctx fields SHOULD only be interpreted by the
 * LSM specified by @id; they MUST be set to zero/0 when not used.
 */
struct lsm_ctx {
	uint64_t id;
	uint64_t flags;
	uint64_t len;
	uint64_t ctx_len;
	uint8_t ctx[];
};
#endif

/*
 * ID tokens to identify Linux Security Modules (LSMs)
 *
 * These token values are used to uniquely identify specific LSMs
 * in the kernel as well as in the kernel's LSM userspace API.
 */
#ifndef LSM_ID_UNDEF
# define LSM_ID_UNDEF		0
#endif

#ifndef LSM_ID_CAPABILITY
# define LSM_ID_CAPABILITY	100
#endif

#ifndef LSM_ID_SELINUX
# define LSM_ID_SELINUX		101
#endif

#ifndef LSM_ID_SMACK
# define LSM_ID_SMACK		102
#endif

#ifndef LSM_ID_TOMOYO
# define LSM_ID_TOMOYO		103
#endif

#ifndef LSM_ID_APPARMOR
# define LSM_ID_APPARMOR	104
#endif

#ifndef LSM_ID_YAMA
# define LSM_ID_YAMA		105
#endif

#ifndef LSM_ID_LOADPIN
# define LSM_ID_LOADPIN		106
#endif

#ifndef LSM_ID_SAFESETID
# define LSM_ID_SAFESETID	107
#endif

#ifndef LSM_ID_LOCKDOWN
# define LSM_ID_LOCKDOWN	108
#endif

#ifndef LSM_ID_BPF
# define LSM_ID_BPF		109
#endif

#ifndef LSM_ID_LANDLOCK
# define LSM_ID_LANDLOCK	110
#endif

#ifndef LSM_ID_IMA
# define LSM_ID_IMA		111
#endif

#ifndef LSM_ID_EVM
# define LSM_ID_EVM		112
#endif

#ifndef LSM_ID_IPE
# define LSM_ID_IPE		113
#endif

/*
 * LSM_ATTR_XXX definitions identify different LSM attributes
 * which are used in the kernel's LSM userspace API. Support
 * for these attributes vary across the different LSMs. None
 * are required.
 */
#ifndef LSM_ATTR_UNDEF
# define LSM_ATTR_UNDEF		0
#endif

#ifndef LSM_ATTR_CURRENT
# define LSM_ATTR_CURRENT	100
#endif

#ifndef LSM_ATTR_EXEC
# define LSM_ATTR_EXEC		101
#endif

#ifndef LSM_ATTR_FSCREATE
# define LSM_ATTR_FSCREATE	102
#endif

#ifndef LSM_ATTR_KEYCREATE
# define LSM_ATTR_KEYCREATE	103
#endif

#ifndef LSM_ATTR_PREV
# define LSM_ATTR_PREV		104
#endif

#ifndef LSM_ATTR_SOCKCREATE
# define LSM_ATTR_SOCKCREATE	105
#endif

/*
 * LSM_FLAG_XXX definitions identify special handling instructions
 * for the API.
 */
#ifndef LSM_FLAG_SINGLE
# define LSM_FLAG_SINGLE	0x0001
#endif

static inline int lsm_get_self_attr(uint32_t attr, struct lsm_ctx *ctx,
				    uint32_t *size, uint32_t flags)
{
	return tst_syscall(__NR_lsm_get_self_attr, attr, ctx, size, flags);
}

static inline int lsm_set_self_attr(uint32_t attr, struct lsm_ctx *ctx,
				    uint32_t size, uint32_t flags)
{
	return tst_syscall(__NR_lsm_set_self_attr, attr, ctx, size, flags);
}

static inline int lsm_list_modules(uint64_t *ids, uint32_t *size, uint32_t flags)
{
	return tst_syscall(__NR_lsm_list_modules, ids, size, flags);
}
#endif
