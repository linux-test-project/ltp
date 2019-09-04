/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Richard Palethorpe <rpalethorpe@suse.com>
 */
/**
 * @file tst_capability.h
 *
 * Limited capability operations without libcap.
 */

#ifndef TST_CAPABILITY_H
#define TST_CAPABILITY_H

#include <stdint.h>

#include "lapi/capability.h"

#define TST_CAP_DROP 1
#define TST_CAP_REQ  (1 << 1)

#define TST_CAP(action, capability) {action, capability, #capability}

struct tst_cap_user_header {
	uint32_t version;
	int pid;
};

struct tst_cap_user_data {
	uint32_t effective;
	uint32_t permitted;
	uint32_t inheritable;
};

struct tst_cap {
	uint32_t action;
	uint32_t id;
	char *name;
};

/**
 * Get the capabilities as decided by hdr.
 *
 * Note that the memory pointed to by data should be large enough to store two
 * structs.
 */
int tst_capget(struct tst_cap_user_header *hdr,
	       struct tst_cap_user_data *data);

/**
 * Set the capabilities as decided by hdr and data
 *
 * Note that the memory pointed to by data should be large enough to store two
 * structs.
 */
int tst_capset(struct tst_cap_user_header *hdr,
	       const struct tst_cap_user_data *data);

/**
 * Add, check or remove a capability
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
 * Add, check or remove a capabilities
 *
 * Takes a NULL terminated array of structs which describe whether some
 * capabilities are needed or not and mask that determines subset of the
 * actions to be performed. Loops over the array and if mask matches the
 * element action it's passed to tst_cap_action().
 */
void tst_cap_setup(struct tst_cap *cap, unsigned int action_mask);

#endif /* TST_CAPABILITY_H */
