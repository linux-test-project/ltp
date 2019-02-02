//SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LAPI_USTAT_H
#define LAPI_USTAT_H

#include <sys/types.h>

#ifdef HAVE_SYS_USTAT_H
# include <sys/ustat.h>
#else
struct ustat {
	daddr_t f_tfree;
	ino_t f_tinode;
	char f_fname[6];
	char f_fpack[6];
};
#endif

#endif /* LAPI_USTAT_H */
