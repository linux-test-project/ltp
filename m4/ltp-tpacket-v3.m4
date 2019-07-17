dnl SPDX-License-Identifier: GPL-2.0-or-later
dnl Copyright (c) 2017 Richard Palethorpe <rpalethorpe@suse.com>

AC_DEFUN([LTP_CHECK_TPACKET_V3],[
AC_CHECK_TYPES([struct tpacket_req3],,,[
#ifdef HAVE_LINUX_IF_PACKET_H
# include <linux/if_packet.h>
#endif
])
])
