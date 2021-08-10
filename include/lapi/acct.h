//SPDX-License-Identifier: GPL-2.0-or-later

#ifndef LAPI_ACCT_H__
#define LAPI_ACCT_H__

#include <sys/types.h>
#include "config.h"

#ifdef HAVE_STRUCT_ACCT_V3
#include <sys/acct.h>
#else

#define ACCT_COMM 16

typedef uint16_t comp_t;

/* Fallback structures to parse the process accounting file */
struct acct {
	char ac_flag;
	uint16_t ac_uid;
	uint16_t ac_gid;
	uint16_t ac_tty;
	uint32_t ac_btime;
	comp_t    ac_utime;
	comp_t    ac_stime;
	comp_t    ac_etime;
	comp_t    ac_mem;
	comp_t    ac_io;
	comp_t    ac_rw;
	comp_t    ac_minflt;
	comp_t    ac_majflt;
	comp_t    ac_swaps;
	uint32_t ac_exitcode;
	char      ac_comm[ACCT_COMM+1];
	char      ac_pad[10];
};

struct acct_v3 {
	char      ac_flag;
	char      ac_version;
	uint16_t ac_tty;
	uint32_t ac_exitcode;
	uint32_t ac_uid;
	uint32_t ac_gid;
	uint32_t ac_pid;
	uint32_t ac_ppid;
	uint32_t ac_btime;
	float     ac_etime;
	comp_t    ac_utime;
	comp_t    ac_stime;
	comp_t    ac_mem;
	comp_t    ac_io;
	comp_t    ac_rw;
	comp_t    ac_minflt;
	comp_t    ac_majflt;
	comp_t    ac_swaps;
	char      ac_comm[ACCT_COMM];
};

/* Possible values for the ac_flag member */
enum {
	AFORK = 0x01,
	ASU   = 0x02,
	ACORE = 0x08,
	AXSIG = 0x10
};
# if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# define ACCT_BYTEORDER  0x80
# elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define ACCT_BYTEORDER  0x00
# endif
#endif /* HAVE_STRUCT_ACCT_V3 */

#endif /* LAPI_ACCT_H__ */
