/*
 * memtoy.h -- local header template for memory toy/tool
 */
/*
 *  Copyright (c) 2005 Hewlett-Packard, Inc
 *  All rights reserved.
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#ifndef _MEMTOY_H
#define _MEMTOY_H
#include <sys/types.h>
#include <sys/time.h>

#include <setjmp.h>
#include <signal.h>

#include "segment.h"
#include "version.h"

#define BOGUS_SIZE ((size_t)-1)

typedef enum {false=0, true} bool;

/*
 * program global data
 */
typedef struct global_context {
	char          *program_name;     /* argv[0] - for reference in messages */

	unsigned long  options;          /* command line options, ... */

	siginfo_t     *siginfo;          /* signal info, if signalled != 0 */
	char          *signame;          /* name of signal, if any */
	sigjmp_buf     sigjmp_env;       /* embedded setjmp buffer */
	bool           sigjmp;           /* sigsetjmp is "armed" */

	size_t         pagesize;         /* system page size for mmap, ... */

	int            numa_max_node;    /* if >0, numa supported */

	segment_t    **seglist;          /* list of known segments */
	segment_t     *seg_avail;        /* an available segment */

	char          *cmd_name;         /* currently executing command */

#ifdef _DEBUG
	unsigned long  debug;            /* debug enablement flags */
#endif
} glctx_t;

extern glctx_t glctx;

#define OPTION_VERBOSE 0x0001
#define OPTION_INTERACTIVE 0x0100

/*
 * Danger, Will Robinson!!  -- hardcoded variable 'gcp'
 */
#define set_option(OPT)  gcp->options |= (OPTION_##OPT)
#define clear_option(OPT)  gcp->options &= ~(OPION_##OPTT)
#define is_option(OPT) ((gcp->options & OPTION_##OPT) != 0)

#define show_option(opt, off, on) ( !is_option(opt) ? #off : #on )

#define signalled(GCP) (GCP->siginfo != NULL)

/*
 * different between start and end time in microseconds
 */
static unsigned long tv_diff_usec(struct timeval *stp, struct timeval *etp)
{
	return ((1000000L * (etp)->tv_sec + (etp)->tv_usec) -
		(1000000L * (stp)->tv_sec + (stp)->tv_usec));
}


/*
 * memtoy.c
 */
extern void die(int, char*, ... );
extern void vprint(char*, ...);
extern void reset_signal(void);
extern void wait_for_signal(const char*);

/*
 * commands.c
 */
extern void process_commands(void);
extern void wait_for_signal(const char *);
extern void touch_memory(bool, unsigned long*, size_t);

#endif
