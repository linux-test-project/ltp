/* watch -- execute a program repeatedly, displaying output fullscreen
 *
 * Based on the original 1991 'watch' by Tony Rems <rembo@unisoft.com>
 * (with mods and corrections by Francois Pinard).
 *
 * Substantially reworked, new features (differences option, SIGWINCH
 * handling, unlimited command length, long line handling) added Apr 1999 by
 * Mike Coleman <mkc@acm.org>.
 *
 * Changes by Albert Cahalan, 2002.
 */

#define VERSION "0.2.0"

#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include "proc/procps.h"

static struct option longopts[] = {
	{"differences", optional_argument, 0, 'd'},
	{"help", no_argument, 0, 'h'},
	{"interval", required_argument, 0, 'n'},
	{"version", no_argument, 0, 'v'},
	{0, 0, 0, 0}
};

static char usage[] =
    "Usage: %s [-dhnv] [--differences[=cumulative]] [--help] [--interval=<n>] [--version] <command>\n";

static char *progname;

static int curses_started = 0;
static int height = 24, width = 80;
static int screen_size_changed = 0;
static int first_screen = 1;

#define min(x,y) ((x) > (y) ? (y) : (x))

static void do_usage(void) NORETURN;
static void do_usage(void)
{
	fprintf(stderr, usage, progname);
	exit(1);
}

static void do_exit(int status) NORETURN;
static void do_exit(int status)
{
	if (curses_started)
		endwin();
	exit(status);
}

/* signal handler */
static void die(int notused) NORETURN;
static void die(int notused)
{
	(void) notused;
	do_exit(0);
}

static void
winch_handler(int notused)
{
	(void) notused;
	screen_size_changed = 1;
}

static void
get_terminal_size(void)
{
	struct winsize w;
	if (ioctl(2, TIOCGWINSZ, &w) == 0) {
		if (w.ws_row > 0)
			height = w.ws_row;
		if (w.ws_col > 0)
			width = w.ws_col;
	}
}

int
main(int argc, char *argv[])
{
	int optc;
	int option_differences = 0,
	    option_differences_cumulative = 0,
	    option_help = 0, option_version = 0;
	int interval = 2;
	char *command;
	int command_length = 0;	/* not including final \0 */

	setlocale(LC_ALL, "");
	progname = argv[0];

	while ((optc = getopt_long(argc, argv, "+d::hn:v", longopts, (int *) 0))
	       != EOF) {
		switch (optc) {
		case 'd':
			option_differences = 1;
			if (optarg)
				option_differences_cumulative = 1;
			break;
		case 'h':
			option_help = 1;
			break;
		case 'n':
			{
				char *str;
				interval = strtol(optarg, &str, 10);
				if (!*optarg || *str)
					do_usage();
			}
			break;
		case 'v':
			option_version = 1;
			break;
		default:
			do_usage();
			break;
		}
	}

	if (option_version) {
		fprintf(stderr, "%s\n", VERSION);
		if (!option_help)
			exit(0);
	}

	if (option_help) {
		fprintf(stderr, usage, progname);
		fputs
		    ("  -d, --differences[=cumulative]\thighlight changes between updates\n",
		     stderr);
		fputs("\t\t(cumulative means highlighting is cumulative)\n",
		      stderr);
		fputs("  -h, --help\t\t\t\tprint a summary of the options\n",
		      stderr);
		fputs
		    ("  -n, --interval=<seconds>\t\tseconds to wait between updates\n",
		     stderr);
		fputs("  -v, --version\t\t\t\tprint the version number\n",
		      stderr);
		exit(0);
	}

	if (optind >= argc)
		do_usage();

	command = strdup(argv[optind++]);
	command_length = strlen(command);
	for (; optind < argc; optind++) {
		char *endp;
		int s = strlen(argv[optind]);
		command = realloc(command, command_length + s + 2);	/* space and \0 */
		endp = command + command_length;
		*endp = ' ';
		memcpy(endp + 1, argv[optind], s);
		command_length += 1 + s;	/* space then string length */
		command[command_length] = '\0';
	}

	get_terminal_size();

	/* Catch keyboard interrupts so we can put tty back in a sane state.  */
	signal(SIGINT, die);
	signal(SIGTERM, die);
	signal(SIGHUP, die);
	signal(SIGWINCH, winch_handler);

	/* Set up tty for curses use.  */
	curses_started = 1;
	initscr();
	nonl();
	noecho();
	cbreak();

	for (;;) {
		time_t t = time(NULL);
		char *ts = ctime(&t);
		int tsl = strlen(ts);
		char *header;
		FILE *p;
		int x, y;
		int oldeolseen = 1;

		if (screen_size_changed) {
			get_terminal_size();
			resizeterm(height, width);
			clear();
			/* redrawwin(stdscr); */
			screen_size_changed = 0;
			first_screen = 1;
		}

		/* left justify interval and command, right justify time, clipping all
		   to fit window width */
		asprintf(&header, "Every %ds: %.*s",
			 interval, min(width - 1, command_length), command);
		mvaddstr(0, 0, header);
		if (strlen(header) > (size_t) (width - tsl - 1))
			mvaddstr(0, width - tsl - 4, "...  ");
		mvaddstr(0, width - tsl + 1, ts);
		free(header);

		if (!(p = popen(command, "r"))) {
			perror("popen");
			do_exit(2);
		}

		for (y = 2; y < height; y++) {
			int eolseen = 0, tabpending = 0;
			for (x = 0; x < width; x++) {
				int c = ' ';
				int attr = 0;

				if (!eolseen) {
					/* if there is a tab pending, just spit spaces until the
					   next stop instead of reading characters */
					if (!tabpending)
						do
							c = getc(p);
						while (c != EOF && !isprint(c)
						       && c != '\n'
						       && c != '\t');
					if (c == '\n')
						if (!oldeolseen && x == 0) {
							x = -1;
							continue;
						} else
							eolseen = 1;
					else if (c == '\t')
						tabpending = 1;
					if (c == EOF || c == '\n' || c == '\t')
						c = ' ';
					if (tabpending && (((x + 1) % 8) == 0))
						tabpending = 0;
				}
				move(y, x);
				if (option_differences) {
					int oldch = inch();
					char oldc = oldch & A_CHARTEXT;
					attr = !first_screen
					    && (c != oldc
						||
						(option_differences_cumulative
						 && (oldch & A_ATTRIBUTES)));
				}
				if (attr)
					standout();
				addch(c);
				if (attr)
					standend();
			}
			oldeolseen = eolseen;
		}

		pclose(p);

		first_screen = 0;
		refresh();
		sleep(interval);
	}

	endwin();

	return 0;
}
