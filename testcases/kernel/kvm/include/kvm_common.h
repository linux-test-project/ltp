/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * Common definitions for communication between KVM guest and host.
 */

#ifndef KVM_COMMON_H_
#define KVM_COMMON_H_

#define KVM_TNONE	-1	/* "No result" status value */

/*
 * Result value indicating end of test. If the test program exits using
 * the HLT instruction with any valid result value other than KVM_TEXIT or
 * TBROK, KVM runner will automatically resume VM execution after printing
 * the message.
 */
#define KVM_TEXIT	0xff

#define KVM_RESULT_BASEADDR 0xfffff000
#define KVM_RESULT_SIZE 0x1000

struct tst_kvm_result {
	int32_t result;
	int32_t lineno;
	uint64_t file_addr;
	char message[0];
};

#endif /* KVM_COMMON_H_ */
