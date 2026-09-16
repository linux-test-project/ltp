// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef KEYCTL_COMMON_H__
#define KEYCTL_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tst_test.h"
#include "lapi/keyctl.h"

#define KEY_PERM_ALL		(KEY_POS_ALL | KEY_USR_ALL | KEY_GRP_ALL | KEY_OTH_ALL)
#define KEY_PERM_SET		(KEY_POS_ALL | KEY_USR_ALL)

#define KEY_VIEW_BITS		(KEY_POS_VIEW | KEY_USR_VIEW | KEY_GRP_VIEW | KEY_OTH_VIEW)
#define KEY_PERM_NO_VIEW	(KEY_PERM_ALL & ~KEY_VIEW_BITS)

#define KEY_WRITE_BITS		(KEY_POS_WRITE | KEY_USR_WRITE | KEY_GRP_WRITE | KEY_OTH_WRITE)
#define KEY_PERM_NO_WRITE	(KEY_PERM_ALL & ~KEY_WRITE_BITS)

#define KEY_SETATTR_BITS	(KEY_POS_SETATTR | KEY_USR_SETATTR | KEY_GRP_SETATTR | KEY_OTH_SETATTR)
#define KEY_PERM_NO_SETATTR	(KEY_PERM_ALL & ~KEY_SETATTR_BITS)

#define SAFE_NEW_RING(desc) \
	safe_add_key(__FILE__, __LINE__, "keyring", (desc), NULL, 0, KEY_SPEC_PROCESS_KEYRING)

#define SAFE_NEW_USER_KEY(desc, payload, plen, ring) \
	safe_add_key(__FILE__, __LINE__, "user", (desc), (payload), (plen), (ring))

static inline bool is_asym_supported(key_serial_t ring_builtin)
{
	TEST(keyctl(KEYCTL_RESTRICT_KEYRING, ring_builtin,
		    (unsigned long)"asymmetric", (unsigned long)"bogus", 0));

	return !(TST_RET == -1 && TST_ERR == ENOKEY);
}

#endif /* KEYCTL_COMMON_H__ */
