/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
 */

#ifndef TST_CPU_H__
#define TST_CPU_H__

long tst_ncpus(void);
long tst_ncpus_conf(void);
long tst_ncpus_max(void);

#define VIRT_ANY	0	/* catch-all argument for tst_is_virt() */
#define VIRT_XEN	1	/* xen dom0/domU */
#define VIRT_KVM	2	/* only default virtual CPU */
#define VIRT_OTHER	0xffff	/* unrecognized hypervisor */

int tst_is_virt(int virt_type);

#endif	/* TST_CPU_H__ */
