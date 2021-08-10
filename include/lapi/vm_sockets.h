// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 SUSE LLC <rpalethorpe@suse.com>
 */

#ifndef LAPI_VM_SOCKETS_H__
#define LAPI_VM_SOCKETS_H__

#include <sys/socket.h>

#if HAVE_LINUX_VM_SOCKETS_H
#  include <linux/vm_sockets.h>
#endif

#ifndef VMADDR_CID_LOCAL
#  define VMADDR_CID_LOCAL 1
#endif

#endif /* LAPI_VM_SOCKETS_H__ */
