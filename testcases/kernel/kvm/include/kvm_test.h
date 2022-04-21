/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2021 SUSE LLC <mdoucha@suse.cz>
 *
 * Minimal test library for KVM tests
 */

#ifndef KVM_TEST_H_
#define KVM_TEST_H_

#ifdef COMPILE_PAYLOAD
# include "kvm_guest.h"
# include "kvm_common.h"
#else
# include "tst_test.h"
# include "kvm_host.h"
#endif /* COMPILE_PAYLOAD */

#endif /* KVM_TEST_H_ */
