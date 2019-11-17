/*
 * memtoy.c -- toy/tool for investigating Linux [Numa] VM behavior
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

#include <stdio.h>

#include "config.h"
#include "tst_res_flags.h"

#ifdef HAVE_NUMA_V2

#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <libgen.h>
#include <errno.h>
#include <numa.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memtoy.h"

/*
 * global context
 */
glctx_t glctx;			/* global context */

/*
 * command line options:
 *
 *  -v          = verbose
 *  -V          = display version
 *  -h|x	= display help.
 */
#define OPTIONS	"Vhvx"

/*
 * usage/help message
 */
char *USAGE = "\nUsage:  %s [-v] [-V] [-{h|x}]\n\n\
Where:\n\
\t-v            enable verbosity\n\
\t-V            display version info\n\
\t-h|x          show this usage/help message\n\
\n\
More info - TODO\n\
";

/*
 * die() - emit error message and exit w/ specified return code.
 *	   if exit_code < 0, save current errno, and fetch associated
 *	   error string.  Print error string after app error message.
 *	   Then exit with abs(exit_code).
 */
void die(int exit_code, char *format, ...)
{
	va_list ap;
	char *errstr;
	int saverrno;

	va_start(ap, format);

	if (exit_code < 0) {
		saverrno = errno;
		errstr = strerror(errno);
	}

	(void)vfprintf(stderr, format, ap);
	va_end(ap);

	if (exit_code < 0)
		fprintf(stderr, "Error = (%d) %s\n", saverrno, errstr);

	exit(abs(exit_code));
}

void usage(char *mesg)
{
	if (mesg != NULL) {
		fprintf(stderr, "%s\n", mesg);
	}
	fprintf(stderr, USAGE, glctx.program_name);
	exit(1);
}

#ifdef _DEBUG
/*
 * This function is a wrapper around "fprintf(stderr, ...)" so that we
 * can use the DPRINTF(<flag>, (<[f]printf arguments>)) macro for debug
 * prints.  See the definition of DPRINTF in XXX.h
 */
int _dvprintf(char *format, ...)
{
	va_list ap;
	int retval;

	va_start(ap, format);

	retval = vfprintf(stderr, format, ap);

	va_end(ap);

	fflush(stderr);
	return (retval);
}
#endif

void vprint(char *format, ...)
{
	va_list ap;
	glctx_t *gcp = &glctx;

	va_start(ap, format);

	if (!is_option(VERBOSE))
		goto out;

	(void)vfprintf(stderr, format, ap);
	fflush(stderr);

out:
	va_end(ap);
	return;

}

/*
 * =========================================================================
 */
static int signals_to_handle[] = {
	SIGINT, SIGQUIT, SIGSEGV, SIGBUS,
	SIGUSR1, SIGUSR2, 0
};

static char *sig_names[] = {
	"SIGINT", "SIGQUIT", "SIGSEGV", "SIGBUS",
	"SIGUSR1", "SIGUSR2", "unknown", 0
};

/*
 * signal_handler()
 *
 * save siginfo and name in global context
 */
void signal_handler(int sig, siginfo_t * info, void *vcontext)
{
	glctx_t *gcp = &glctx;
	int isig = 0, *sigp = signals_to_handle;
	static siginfo_t infocopy;

	/*
	 * static copy of signal info.
	 * Note, additional signals, before use, can overwrite
	 */
	infocopy = *info;
	gcp->siginfo = &infocopy;

	while (*sigp) {
		if (*sigp == sig)
			break;
		++isig;
		++sigp;
	}
	gcp->signame = sig_names[isig];

	vprint("signal hander entered for sig %s\n", gcp->signame);

	switch (sig) {
	case SIGSEGV:
	case SIGBUS:
		if (gcp->sigjmp) {
			gcp->sigjmp = false;
			siglongjmp(gcp->sigjmp_env, 1);
		}

		die(8, "\n%s:  signal %s, but siglongjmp not armed\n",
		    gcp->program_name, gcp->signame);
		break;

	case SIGINT:
	case SIGQUIT:
		break;

	default:
		die(8, "\n%s:  Unexpected signal:  %d\n",
		    gcp->program_name, sig);
		break;
	}
}

/*
 * set_signals()
 *
 * Setup signal dispositions to catch selected signals
 */
void set_signals()
{
	glctx_t *gcp = &glctx;
	int *sigp = signals_to_handle;
	char **namep = sig_names;

	struct sigaction act = {
		.sa_sigaction = signal_handler,
		.sa_flags = SA_SIGINFO
	};

	(void)sigfillset(&(act.sa_mask));

	while (*sigp) {
		char *sig_name = *(namep++);
		int sig = *(sigp++);

		if (0 != sigaction(sig, &act, NULL)) {
			die(-1, "%s: Failed to set sigaction for %s\n",
			    gcp->program_name, sig_name);
		} else
#if 0
			vprint("%s: established handler for %s\n",
			       gcp->program_name, sig_name)
#endif
			    ;
	}

	return;
}

void reset_signal(void)
{
//TODO:  free siginfo if/when malloc'd
	glctx.siginfo = NULL;
	glctx.sigjmp = false;
}

void wait_for_signal(const char *mesg)
{
	printf("%s ... ", mesg);
	fflush(stdout);
	pause();
	vprint("%s: wakened by signal %s\n", __FUNCTION__, glctx.signame);
	reset_signal();
	printf("\n");
	fflush(stdout);
}

void show_siginfo()
{
	glctx_t *gcp = &glctx;
	siginfo_t *info = gcp->siginfo;
	void *badaddr = info->si_addr;
	char *sigcode;

	switch (info->si_signo) {
	case SIGSEGV:
		switch (info->si_code) {
		case SEGV_MAPERR:
			sigcode = "address not mapped";
			break;

		case SEGV_ACCERR:
			sigcode = "invalid access error";
			break;

		default:
			sigcode = "unknown";
			break;
		}
		break;

	case SIGBUS:
		switch (info->si_code) {
		case BUS_ADRALN:
			sigcode = "invalid address alignment";
			break;

		case BUS_ADRERR:
			sigcode = "non-existent physical address";
			break;

		default:
			sigcode = "unknown";
			break;
		}
		break;

	default:
		/*
		 * ignore SIGINT/SIGQUIT
		 */
		return;
	}

	printf("Signal %s @ 0x%lx - %s\n", gcp->signame, badaddr, sigcode);

}

/*
 * =========================================================================
 */

void touch_memory(bool rw, unsigned long *memp, size_t memlen)
{
	glctx_t *gcp = &glctx;

	unsigned long *memend, *pp, sink;
	unsigned long longs_in_page = gcp->pagesize / sizeof(unsigned long);

	memend = memp + memlen / sizeof(unsigned long);
	vprint("!!!%s from 0x%lx thru 0x%lx\n",
	       rw ? "Writing" : "Reading", memp, memend);

	for (pp = memp; pp < memend; pp += longs_in_page) {
		// vprint("%s:  touching 0x%lx\n", __FUNCTION__, pp);
		if (!sigsetjmp(gcp->sigjmp_env, true)) {
			gcp->sigjmp = true;

			/*
			 *  Mah-ahm!  He's touching me!
			 */
			if (rw)
				*pp = (unsigned long)pp;
			else
				sink = *pp;

			gcp->sigjmp = false;
		} else {
			show_siginfo();
			reset_signal();
			break;
		}

		/*
		 * Any [handled] signal breaks the loop
		 */
		if (gcp->siginfo != NULL) {
			reset_signal();
			break;
		}
	}
}

/*
 * =========================================================================
 */

void init_glctx(glctx_t * gcp)
{

	memset(gcp, 0, sizeof(glctx_t));

	gcp->pagesize = (size_t) sysconf(_SC_PAGESIZE);

	if (numa_available() >= 0) {
		gcp->numa_max_node = numa_max_node();
	} else
		gcp->numa_max_node = -1;

	segment_init(gcp);

	if (isatty(fileno(stdin)))
		set_option(INTERACTIVE);

}

/*
 * cleanup() - at exit cleanup routine
 */
static void cleanup()
{
	glctx_t *gcp = &glctx;

	segment_cleanup(gcp);
}				/* cleanup() */

int parse_command_line_args(int argc, char *argv[])
{
	extern int optind;
	extern char *optarg;

	glctx_t *gcp = &glctx;
	int argval;
	int error = 0;

	char c;

	gcp->program_name = basename(argv[0]);

	/*
	 * process command line options.
	 */
	while ((c = getopt(argc, argv, OPTIONS)) != (char)EOF) {
		char *next;

		switch (c) {

		case 'v':
			set_option(VERBOSE);
			break;

		case 'h':
		case 'x':
			usage(NULL);

			break;

		case 'V':
			printf("memtoy " MEMTOY_VERSION " built "
			       __DATE__ " @ " __TIME__ "\n");
			exit(0);
			break;

#ifdef _DEBUG
		case '0':
			argval = strtoul(optarg, &next, 0);
			if (*next != '\0') {
				fprintf(stderr,
					"-D <debug-mask> must be unsigned hex/decimal integer\n");
				++error;
			} else
				gcp->debug = argval;
			break;
#endif

		default:
			error = 1;
			break;
		}
	}
done:

	return (error);
}

int main(int argc, char *argv[])
{
	glctx_t *gcp = &glctx;
	bool user_is_super;
	int error;

	init_glctx(gcp);
	if (!is_option(INTERACTIVE))
		setbuf(stdout, NULL);

	/*
	 * Register cleanup handler
	 */
	if (atexit(cleanup) != 0) {
		die(-1, "%s:  atexit(cleanup) registration failed\n", argv[0]);
	}

	user_is_super = (geteuid() == 0);

	error = parse_command_line_args(argc, argv);

	if (error /* || argc==1 */ ) {
		usage(NULL);

	}

	/*
	 * actual program logic starts here
	 */
	printf("memtoy pid:  %d\n", getpid());
	vprint("%s:  pagesize = %d\n", gcp->program_name, gcp->pagesize);
	if (gcp->numa_max_node >= 0)
		vprint("%s:  NUMA available - max node: %d\n",
		       gcp->program_name, gcp->numa_max_node);

	set_signals();

	process_commands();

	return 0;

}
#else
int main(void)
{
	fprintf(stderr, NUMA_ERROR_MSG "\n");
	return TCONF;
}
#endif
