/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 */

/**
 * DOC: Capabilities introduction
 *
 * Limited capability operations without libcap.
 */

#ifndef TST_CAPABILITY_H
#define TST_CAPABILITY_H

#include <stdint.h>

#include "lapi/capability.h"

/**
 * enum tst_cap_act - A capability action masks.
 *
 * @TST_CAP_DROP: Drop capabilities.
 * @TST_CAP_REQ: Add capabilities.
 */
enum tst_cap_act {
	TST_CAP_DROP = 1,
	TST_CAP_REQ = (1 << 1)
};

/**
 * struct tst_cap_user_header - Kernel capget(), capset() syscall header.
 *
 * @version: A capability API version.
 * @pid: A process to operate on.
 */
struct tst_cap_user_header {
	uint32_t version;
	int pid;
};

/**
 * struct tst_cap_user_data - Kernel capset(), capget() syscall payload.
 *
 * @effective: A capability effective set.
 * @permitted: A capability permitted set.
 * @inheritable: A capability inheritable set.
 */
struct tst_cap_user_data {
	uint32_t effective;
	uint32_t permitted;
	uint32_t inheritable;
};

/**
 * struct tst_cap - A capability to alter.
 *
 * @action: What should we do, i.e. drop or add a capability.
 * @id: A capability id.
 * @name: A capability name.
 *
 * This structure is usually constructed with the TST_CAP() macro so that the
 * name is created automatically.
 */
struct tst_cap {
	uint32_t action;
	uint32_t id;
	char *name;
};

/**
 * TST_CAP() - Create a struct tst_cap entry.
 *
 * @action: What should we do, i.e. drop or add capability.
 * @capability: A capability id, e.g. CAP_BPF.
 */
#define TST_CAP(action, capability) {action, capability, #capability}

/**
 * tst_capget() - Get the capabilities as decided by hdr.
 *
 * @hdr: A capability user header stores a pid to operate on and which
 *       capability API version is used.
 * @data: A memory to store the capabilities to. The memory pointed to by data
 *        should be large enough to store two structs.
 *
 * return: Returns 0 on success, -1 on a failure and sets errno.
 */
int tst_capget(struct tst_cap_user_header *hdr,
	       struct tst_cap_user_data *data);

/**
 * tst_capset() - Set the capabilities as decided by hdr and data
 *
 * @hdr: A capability user header stores a pid to operate on and which
 *       capability API version is used.
 * @data: A memory to store the capabilities to. The memory pointed to by data
 *        should be large enough to store two structs.
 *
 * return: Returns 0 on success, -1 on a failure and sets errno.
 */
int tst_capset(struct tst_cap_user_header *hdr,
	       const struct tst_cap_user_data *data);

/**
 * tst_cap_action() - Add, check or remove a capability.
 *
 * @cap: An {} terminated array of capabilities to alter.
 *
 * It will attempt to drop or add capability to the effective set. It will
 * try to detect if this is needed and whether it can or can't be done. If it
 * clearly can not add a privilege to the effective set then it will return
 * TCONF. However it may fail for some other reason and return TBROK.
 *
 * This only tries to change the effective set. Some tests may need to change
 * the inheritable and ambient sets, so that child processes retain some
 * capability.
 */
void tst_cap_action(struct tst_cap *cap);


/**
 * tst_cap_setup() - Add, check or remove a capabilities.
 *
 * @cap: An {} terminated array of capabilities to alter.
 * @action_mask: Decides which actions are done, i.e. only drop caps, add them
 *               or both.
 *
 * Takes a NULL terminated array of structs which describe whether some
 * capabilities are needed or not and mask that determines subset of the
 * actions to be performed. Loops over the array and if mask matches the
 * element action it's passed to tst_cap_action().
 */
void tst_cap_setup(struct tst_cap *cap, enum tst_cap_act action_mask);

#endif /* TST_CAPABILITY_H */
