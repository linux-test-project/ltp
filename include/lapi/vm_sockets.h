// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

#ifndef LAPI_VM_SOCKETS_H__
#define LAPI_VM_SOCKETS_H__

#include <sys/socket.h>
#include "config.h"

#if HAVE_LINUX_VM_SOCKETS_H
#  include <linux/vm_sockets.h>
#endif

#ifndef VMADDR_CID_LOCAL
#  define VMADDR_CID_LOCAL 1
#endif

#ifndef HAVE_STRUCT_SOCKADDR_VM
struct sockaddr_vm {
	unsigned short svm_family;
	unsigned short svm_reserved1;
	unsigned int svm_port;
	unsigned int svm_cid;
	unsigned char svm_flags;
	unsigned char svm_zero[sizeof(struct sockaddr) -
			       sizeof(sa_family_t) -
			       sizeof(unsigned short) -
			       sizeof(unsigned int) -
			       sizeof(unsigned int) -
			       sizeof(unsigned char)];
};
#endif

#endif /* LAPI_VM_SOCKETS_H__ */
