/* emacs settings:  -*- c-basic-offset: 8 tab-width: 8 -*-
 *
 * pgrep/pkill -- utilities to filter the process table
 *
 * Copyright 2000 Kjetil Torgrim Homme <kjetilho@ifi.uio.no>
 *
 * May be distributed under the conditions of the
 * GNU General Public License; a copy is in COPYING
 *
 * Changes by Albert Cahalan, 2002.
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>
#include <errno.h>

#include "proc/readproc.h"
#include "proc/sig.h"
#include "proc/devname.h"
#include "proc/sysinfo.h"
#include "proc/version.h" /* procps_version */

static int i_am_pkill = 0;
static const char *progname = "pgrep";

union el {
	long	num;
	char *	str;
};

/* User supplied arguments */

static int opt_full = 0;
static int opt_long = 0;
static int opt_oldest = 0;
static int opt_newest = 0;
static int opt_negate = 0;
static int opt_exact = 0;
static int opt_signal = SIGTERM;

static const char *opt_delim = "\n";
static union el *opt_pgrp = NULL;
static union el *opt_rgid = NULL;
static union el *opt_ppid = NULL;
static union el *opt_sid = NULL;
static union el *opt_term = NULL;
static union el *opt_euid = NULL;
static union el *opt_ruid = NULL;
static char *opt_pattern = NULL;


static int usage (int opt) NORETURN;
static int usage (int opt)
{
	if (i_am_pkill)
		fprintf (stderr, "Usage: pkill [-SIGNAL] [-fvx] ");
	else
		fprintf (stderr, "Usage: pgrep [-flvx] [-d DELIM] ");
	fprintf (stderr, "[-n|-o] [-P PPIDLIST] [-g PGRPLIST] [-s SIDLIST]\n"
		 "\t[-u EUIDLIST] [-U UIDLIST] [-G GIDLIST] [-t TERMLIST] "
		 "[PATTERN]\n");
	exit (opt == '?' ? 0 : 2);
}


static union el *
split_list (const char *restrict str, int (*convert)(const char *, union el *))
{
	char *copy = strdup (str);
	char *ptr = copy;
	char *sep_pos;
	int i = 1, size = 32;
	union el *list;

	list = malloc (size * sizeof (union el));
	if (list == NULL)
		exit (3);

	do {
		sep_pos = strchr (ptr, ',');
		if (sep_pos)
			*sep_pos = 0;
		if (convert (ptr, &list[i]))
			++i;
		else
			exit (2);
		if (i == size) {
			size *= 2;
			list = realloc (list, size * sizeof (union el));
			if (list == NULL)
				exit (3);
		}
		if (sep_pos)
			ptr = sep_pos + 1;
	} while (sep_pos);

	free (copy);
	if (i == 1) {
		free (list);
		list = NULL;
	} else {
		list[0].num = i - 1;
	}
	return (list);
}

/* strict_atol returns a Boolean: TRUE if the input string contains a
   plain number, FALSE if there are any non-digits. */

static int
strict_atol (const char *restrict str, long *restrict value)
{
	int res = 0;
	int sign = 1;

	if (*str == '+')
		++str;
	else if (*str == '-') {
		++str;
		sign = -1;
	}

	for ( ; *str; ++str) {
		if (! isdigit (*str))
			return (0);
		res *= 10;
		res += *str - '0';
	}
	*value = sign * res;
	return (1);
}

static int
conv_uid (const char *restrict name, union el *restrict e)
{
	struct passwd *pwd;

	if (strict_atol (name, &e->num))
		return (1);

	pwd = getpwnam (name);
	if (pwd == NULL) {
		fprintf (stderr, "%s: invalid user name: %s\n",
			 progname, name);
		return (0);
	}
	e->num = pwd->pw_uid;
	return (1);
}


static int
conv_gid (const char *restrict name, union el *restrict e)
{
	struct group *grp;

	if (strict_atol (name, &e->num))
		return (1);

	grp = getgrnam (name);
	if (grp == NULL) {
		fprintf (stderr, "%s: invalid group name: %s\n",
			 progname, name);
		return (0);
	}
	e->num = grp->gr_gid;
	return (1);
}


static int
conv_pgrp (const char *restrict name, union el *restrict e)
{
	if (! strict_atol (name, &e->num)) {
		fprintf (stderr, "%s: invalid process group: %s\n",
			 progname, name);
		return (0);
	}
	if (e->num == 0)
		e->num = getpgrp ();
	return (1);
}


static int
conv_sid (const char *restrict name, union el *restrict e)
{
	if (! strict_atol (name, &e->num)) {
		fprintf (stderr, "%s: invalid session id: %s\n",
			 progname, name);
		return (0);
	}
	if (e->num == 0)
		e->num = getsid (0);
	return (1);
}


static int
conv_num (const char *restrict name, union el *restrict e)
{
	if (! strict_atol (name, &e->num)) {
		fprintf (stderr, "%s: not a number: %s\n",
			 progname, name);
		return (0);
	}
	return (1);
}


static int
conv_str (const char *restrict name, union el *restrict e)
{
	e->str = strdup (name);
	return (1);
}


static int
match_numlist (long value, const union el *restrict list)
{
	int found = 0;
	if (list == NULL)
		found = 0;
	else {
		int i;
		for (i = list[0].num; i > 0; i--) {
			if (list[i].num == value)
				found = 1;
		}
	}
	return (found);
}

static int
match_strlist (const char *restrict value, const union el *restrict list)
{
	int found = 0;
	if (list == NULL)
		found = 0;
	else {
		int i;
		for (i = list[0].num; i > 0; i--) {
			if (! strcmp (list[i].str, value))
				found = 1;
		}
	}
	return (found);
}

static void
output_numlist (const union el *restrict list)
{
	int i;
	for (i = 1; i < list[0].num; i++)
		printf ("%ld%s", list[i].num, opt_delim);

	if (list[0].num)
		printf ("%ld\n", list[i].num);
}

static void
output_strlist (const union el *restrict list)
{
// FIXME: escape codes
	int i;
	for (i = 1; i < list[0].num; i++)
		printf ("%s%s", list[i].str, opt_delim);

	if (list[0].num)
		printf ("%s\n", list[i].str);
}

static PROCTAB *
do_openproc (void)
{
	PROCTAB *ptp;
	int flags = 0;

	if (opt_pattern || opt_full)
		flags |= PROC_FILLCOM;
	if (opt_ruid || opt_rgid)
		flags |= PROC_FILLSTATUS;
	if (opt_oldest || opt_newest || opt_pgrp || opt_sid || opt_term)
		flags |= PROC_FILLSTAT;
	if (!(flags & PROC_FILLSTAT))
		flags |= PROC_FILLSTATUS;  // FIXME: need one, and PROC_FILLANY broken
	if (opt_euid && !opt_negate) {
		int num = opt_euid[0].num;
		int i = num;
		uid_t *uids = malloc (num * sizeof (uid_t));
		if (uids == NULL)
			exit (3);
		while (i-- > 0) {
			uids[i] = opt_euid[i+1].num;
		}
		flags |= PROC_UID;
		ptp = openproc (flags, uids, num);
	} else {
		ptp = openproc (flags);
	}
	return (ptp);
}

static regex_t *
do_regcomp (void)
{
	regex_t *preg = NULL;

	if (opt_pattern) {
		char *re;
		char errbuf[256];
		int re_err;

		preg = malloc (sizeof (regex_t));
		if (preg == NULL)
			exit (3);
		if (opt_exact) {
			re = malloc (strlen (opt_pattern) + 5);
			if (re == NULL)
				exit (3);
			sprintf (re, "^(%s)$", opt_pattern);
		} else {
		 	re = opt_pattern;
		}

		re_err = regcomp (preg, re, REG_EXTENDED | REG_NOSUB);
		if (re_err) {
			regerror (re_err, preg, errbuf, sizeof(errbuf));
			fprintf (stderr, errbuf);
			exit (2);
		}
	}
	return preg;
}

static union el *
select_procs (void)
{
	PROCTAB *ptp;
	proc_t task;
	unsigned long long saved_start_time;      // for new/old support
	pid_t saved_pid = 0;                      // for new/old support
	int matches = 0;
	int size = 32;
	regex_t *preg;
	pid_t myself = getpid();
	union el *list;
	char cmd[4096];

	list = malloc (size * sizeof (union el));
	if (list == NULL)
		exit (3);

	ptp = do_openproc ();
	preg = do_regcomp ();

	if (opt_newest) saved_start_time =  0ULL;
	if (opt_oldest) saved_start_time = ~0ULL;
	if (opt_newest) saved_pid = 0;
	if (opt_oldest) saved_pid = INT_MAX;
	
	memset (&task, 0, sizeof (task));
	while (readproc (ptp, &task)) {
		int match = 1;

		if (task.pid == myself)
			continue;
		else if (opt_newest && task.start_time < saved_start_time)
			match = 0;
		else if (opt_oldest && task.start_time > saved_start_time)
			match = 0;
		else if (opt_ppid && ! match_numlist (task.ppid, opt_ppid))
			match = 0;
		else if (opt_pgrp && ! match_numlist (task.pgrp, opt_pgrp))
			match = 0;
		else if (opt_euid && ! match_numlist (task.euid, opt_euid))
			match = 0;
		else if (opt_ruid && ! match_numlist (task.ruid, opt_ruid))
			match = 0;
		else if (opt_rgid && ! match_numlist (task.rgid, opt_rgid))
			match = 0;
		else if (opt_sid && ! match_numlist (task.session, opt_sid))
			match = 0;
		else if (opt_term) {
			if (task.tty == 0) {
				match = 0;
			} else {
				char tty[256];
				dev_to_tty (tty, sizeof(tty) - 1,
					    task.tty, task.pid, ABBREV_DEV);
				match = match_strlist (tty, opt_term);
			}
		}
		if (opt_long || (match && opt_pattern)) {
			if (opt_full && task.cmdline) {
				int i = 0;
				int bytes = sizeof (cmd) - 1;

				/* make sure it is always NUL-terminated */
				cmd[bytes] = 0;
				/* make room for SPC in loop below */
				--bytes;

				strncpy (cmd, task.cmdline[i], bytes);
				bytes -= strlen (task.cmdline[i++]);
				while (task.cmdline[i] && bytes > 0) {
					strncat (cmd, " ", bytes);
					strncat (cmd, task.cmdline[i], bytes);
					bytes -= strlen (task.cmdline[i++]) + 1;
				}
			} else {
				strcpy (cmd, task.cmd);
			}
		}

		if (match && opt_pattern) {
			if (regexec (preg, cmd, 0, NULL, 0) != 0)
				match = 0;
		}

		if (match ^ opt_negate) {	/* Exclusive OR is neat */
			if (opt_newest) {
				if (saved_start_time == task.start_time &&
				    saved_pid > task.pid)
					continue;
				saved_start_time = task.start_time;
				saved_pid = task.pid;
				matches = 0;
			}
			if (opt_oldest) {
				if (saved_start_time == task.start_time &&
				    saved_pid < task.pid)
					continue;
				saved_start_time = task.start_time;
				saved_pid = task.pid;
				matches = 0;
			}
			if (opt_long) {
				char buff[5096];  // FIXME
				sprintf (buff, "%d %s", task.pid, cmd);
				list[++matches].str = strdup (buff);
			} else {
				list[++matches].num = task.pid;
			}
			if (matches == size) {
				size *= 2;
				list = realloc (list,
						size * sizeof (union el));
				if (list == NULL)
					exit (3);
			}
		}
		
		memset (&task, 0, sizeof (task));
	}
	closeproc (ptp);

	list[0].num = matches;
	return (list);
}


static void
parse_opts (int argc, char **argv)
{
	char opts[32] = "";
	int opt;
	int criteria_count = 0;

	if (strstr (argv[0], "pkill")) {
		i_am_pkill = 1;
		progname = "pkill";
		/* Look for a signal name or number as first argument */
		if (argc > 1 && argv[1][0] == '-') {
			int sig;
			sig = signal_name_to_number (argv[1] + 1);
			if (sig == -1 && isdigit (argv[1][1]))
				sig = atoi (argv[1] + 1);
			if (sig != -1) {
				int i;
				for (i = 2; i < argc; i++)
					argv[i-1] = argv[i];
				--argc;
				opt_signal = sig;
			}
		}
	} else {
		/* These options are for pgrep only */
		strcat (opts, "ld:");
	}
			
	strcat (opts, "fnovxP:g:s:u:U:G:t:?V");
	
	while ((opt = getopt (argc, argv, opts)) != -1) {
		switch (opt) {
		case 'f':
			opt_full = 1;
			break;
		case 'l':
			opt_long = 1;
			break;
		case 'n':
			if (opt_oldest|opt_negate|opt_newest)
				usage (opt);
			opt_newest = 1;
			++criteria_count;
			break;
		case 'o':
			if (opt_oldest|opt_negate|opt_newest)
				usage (opt);
			opt_oldest = 1;
			++criteria_count;
			break;
		case 'v':
			if (opt_oldest|opt_negate|opt_newest)
				usage (opt);
	  		opt_negate = 1;
			break;
		case 'x':
			opt_exact = 1;
			break;
		case 'd':
			opt_delim = strdup (optarg);
			break;
		case 'P':
	  		opt_ppid = split_list (optarg, conv_num);
			if (opt_ppid == NULL)
				usage (opt);
			++criteria_count;
			break;
		case 'g':
	  		opt_pgrp = split_list (optarg, conv_pgrp);
			if (opt_pgrp == NULL)
				usage (opt);
			break;
		case 's':
	  		opt_sid = split_list (optarg, conv_sid);
			if (opt_sid == NULL)
				usage (opt);
			++criteria_count;
			break;
		case 'u':
	  		opt_euid = split_list (optarg, conv_uid);
			if (opt_euid == NULL)
				usage (opt);
			++criteria_count;
			break;
		case 'U':
	  		opt_ruid = split_list (optarg, conv_uid);
			if (opt_ruid == NULL)
				usage (opt);
			++criteria_count;
			break;
		case 'G':
	  		opt_rgid = split_list (optarg, conv_gid);
			if (opt_rgid == NULL)
				usage (opt);
			++criteria_count;
			break;
		case 't':
	  		opt_term = split_list (optarg, conv_str);
			if (opt_term == NULL)
				usage (opt);
			++criteria_count;
			break;
		case '?':
			usage (opt);
			break;
		case 'V':
			fprintf(stdout, "%s (%s)\n", progname, procps_version);
			exit(0);
		}
	}
        if (argc - optind == 1)
		opt_pattern = argv[optind];
	else if (argc - optind > 1)
		usage (0);
	else if (criteria_count == 0) {
		fprintf (stderr, "%s: No matching criteria specified\n",
			 progname);
		usage (0);
	}
}


int
main (int argc, char **argv)
{
	union el *procs;

	parse_opts (argc, argv);

	procs = select_procs ();
	if (i_am_pkill) {
		int i;
		for (i = 1; i <= procs[0].num; i++) {
			if (kill (procs[i].num, opt_signal) == -1)
				fprintf (stderr, "pkill: %ld - %s\n",
					 procs[i].num, strerror (errno));
		}
	} else {
		if (opt_long)
			output_strlist (procs);
		else
			output_numlist (procs);
	}
	return ((procs[0].num) == 0 ? 1 : 0);
}
