/*
	* Copyright (C) Bull S.A. 2001
	* Copyright (c) International Business Machines  Corp., 2001
	*
	*   This program is free software;  you can redistribute it and/or modify
	*   it under the terms of the GNU General Public License as published by
	*   the Free Software Foundation; either version 2 of the License, or
	*   (at your option) any later version.
	*
	*   This program is distributed in the hope that it will be useful,
	*   but WITHOUT ANY WARRANTY;  without even the implied warranty of
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
	*   the GNU General Public License for more details.
	*
	*   You should have received a copy of the GNU General Public License
	*   along with this program;  if not, write to the Free Software
	*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
	*/

/******************************************************************************/
/*									    */
/* Dec-03-2001  Created: Jacky Malcles & Jean Noel Cordenner		  */
/*	      These tests are adapted from AIX float PVT tests.	     */
/*									    */
/******************************************************************************/
#include "tfloat.h"

#include "test.h"

#define SAFE_FREE(p) { if (p) { free(p); (p)=NULL; } }
/* LTP status reporting */
static char *TCID;			/* Test program identifier.    */
static int TST_TOTAL = 1;		/* Total number of test cases. */

/* To avoid extensive modifications to the code, use this bodge */
#define exit(x) myexit(x)

static void myexit(int x)
{
	if (x)
		tst_resm(TFAIL, "Test failed");
	else
		tst_resm(TPASS, "Test passed");
	tst_exit();
}

static TH_DATA *pcom;
static TH_DATA **tabcom;
static TH_DATA **tabcour;
#ifndef	PATH_MAX
#define PATH_MAX		1024
#endif
static char datadir[PATH_MAX];		/* DATA directory */

#ifndef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX	1024
#endif
#define DEFAULT_NUM_THREADS	20
static int num_threads = DEFAULT_NUM_THREADS;
static int num_loops = 500;

static int sig_cancel = 0;		/* flag set by handle_signals to tell initial thread
				   to stop creating new threads (signal caught) */

static int indice = 0;			/* # of threads created, to be canceled by handle_signals
				   or waited for by initial thread */

static pthread_mutex_t sig_mutex;
static pthread_t *threads;

static int debug = 0;
static int is_true = 1;

static void *handle_signals(void *);

static void sys_error(const char *, int);

static const double EPS = 0.1e-300;

static const int nb_func = NB_FUNC;

static int generate(char *datadir, char *bin_path)
{
	char *cmdline;
	const char *fmt = "cd %s; %s/%s %s";

	cmdline = (char *)malloc(2 * strlen(bin_path) + strlen(datadir) + strlen(GENERATOR) + strlen(fmt));
	if (cmdline == NULL)
		return (1);
	sprintf(cmdline, fmt, datadir, bin_path, GENERATOR, bin_path);
	system(cmdline);
	free(cmdline);
	return (0);
}

static void cleanup(void)
{
	tst_rmdir();
}

int main(int argc, char *argv[])
{
	int opt = 0;
	pid_t pid;

	char *bin_path, *ltproot;
	void *exit_value;
	pthread_attr_t newattr;
	pthread_t sig_hand;
	size_t stacksize = 2093056;
	int th_num;
	int retvalend = 0;
	int retval = 0;
	int error = 0;
	/*int time=1; */
	int i;

	/* Generate test ID from invocation name */
	if ((TCID = strrchr(argv[0], '/')) != NULL)
		TCID++;
	else
		TCID = argv[0];
	fprintf(stderr, "main %s\n", TCID);
	ltproot = getenv("LTPROOT");
	if (ltproot == NULL || strlen(ltproot) == 0) {
		tst_brkm(TBROK, NULL,
			 "You must set $LTPROOT before executing this test");
	}
	bin_path = (char*)malloc(strlen(ltproot) + 16);
	if (bin_path == NULL) {
		tst_brkm(TBROK | TERRNO, NULL, "malloc failed");
	}
	sprintf(bin_path, "%s/testcases/bin", ltproot);

	tst_tmpdir();

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	datadir[0] = '.';
	datadir[1] = '\0';

	if (argc != 1) {
		while ((opt = getopt(argc, argv, "vn:l:D:?")) != EOF) {
			switch (opt) {
			case 'D':
				strncpy(datadir, optarg, PATH_MAX);
				break;
			case 'l':
				num_loops = atoi(optarg);
				break;
			case 'n':
				num_threads = atoi(optarg);
				break;
			case 'v':
				++debug;	/* verbose mode */
				break;
			default:
				fprintf(stderr,
					"usage: %s [-n number_of_threads] [-v]\n",
					argv[0]);
				fprintf(stderr, "[-l number_of_loops] ");
				fprintf(stderr, "[-D DATAs absolute path]\n");
				exit(1);
			}
		}
	}
	switch (pid = fork()) {
	case -1:
		tst_brkm(TBROK | TERRNO, cleanup, "fork failed");
	case 0:
		generate(datadir, bin_path);
		exit(0);
	default:
		waitpid(pid, NULL, 0);
	}
	SAFE_FREE(bin_path);

	if (debug) {
		tst_resm(TINFO,
			 "%s: will run for %d loops; using %s as a data directory",
			 argv[0], num_loops, datadir);
	}
	if (num_threads <= 0) {
		tst_resm(TWARN,
			 "num_threads undefined or incorrect, using \"1\"");
		num_threads = 1;
	}

	if (nb_func * num_threads > PTHREAD_THREADS_MAX - 2)
		while (nb_func * num_threads > PTHREAD_THREADS_MAX - 2)
			num_threads--;
	if (debug)
		tst_resm(TINFO,
			 "%s: will run %d functions, %d threads per function",
			 argv[0], nb_func, num_threads);

	retval = pthread_mutex_init(&sig_mutex, NULL);
	if (retval != 0)
		sys_error("main : mutex_init(&sig_mutex) FAILED", __LINE__);

	retval = pthread_create(&sig_hand, NULL, handle_signals, NULL);
	if (retval != 0)
		sys_error("main : create(&sig_hand) FAILED", __LINE__);

	/*
	 * Start all calculation threads...
	 */
	threads = (pthread_t *)malloc(nb_func * num_threads * sizeof(pthread_t));
	if (threads == NULL)
		tst_brkm(TFAIL | TERRNO, cleanup, "malloc failed");

	tabcom = (TH_DATA **)malloc((sizeof(TH_DATA *) * nb_func * num_threads));
	if (!tabcom)
		tst_brkm(TFAIL | TERRNO, cleanup, "malloc failed");
	tabcour = tabcom;

	retval = pthread_attr_init(&newattr);
	if (retval != 0)
		sys_error("main : attr_init(&newattr) FAILED", __LINE__);

	if (pthread_attr_setstacksize(&newattr, stacksize))
		sys_error("main: pthread_attr_setstacksize failed", __LINE__);

	retval = pthread_attr_setdetachstate(&newattr, PTHREAD_CREATE_JOINABLE);
	if (retval != 0)
		sys_error("main : attr_setdetachstate(&newattr) FAILED",
			  __LINE__);

	/* run the nb_func functions on num_threads */

	indice = 0;
	for (i = 0; i < nb_func; i++) {
		tst_resm(TINFO, "  > running test %d", i+1);
		for (th_num = 0; th_num < num_threads; th_num++) {

			/* allocate struct of commucation  with the thread */
			pcom = (TH_DATA *)calloc(1, sizeof(TH_DATA));
			if (pcom == NULL)
				tst_brkm(TFAIL | TERRNO, cleanup,
					 "calloc failed");
			*tabcour = (TH_DATA *) pcom;
			tabcour++;
			/*
			 * update structure of communication
			 */
			pcom->th_num = th_num;
			pcom->th_func = th_func[i];

			pthread_mutex_lock(&sig_mutex);

			if (sig_cancel) {	/* stop processing right now! */
				pthread_mutex_unlock(&sig_mutex);
				goto finished;
			}
			retval = pthread_create(&threads[indice], &newattr,
						thread_code, (void *)pcom);
			if (retval != 0)
				sys_error("main : create FAILED", __LINE__);
			indice++;
			pthread_mutex_unlock(&sig_mutex);

		}		/* num_threads */
	}			/* for i */

	/*alarm(60*time); *//* start all threads for TEST_time */

	/*
	 * Wait for the threads finish their task
	 * pthread_join () will block
	 */

finished:
	if (debug) {
		tst_resm(TINFO,
			 "initial thread: Waiting for %d threads to finish",
			 indice);
	}
	tabcour = tabcom;

	for (th_num = 0; th_num < indice; th_num++) {
		retvalend = pthread_join(threads[th_num], &exit_value);
		if (retvalend != 0)
			sys_error("finish : join FAILED", __LINE__);

		/* test the result in TH_DATA : communication buffer */
		pcom = *tabcour++;
		if (pcom->th_result != 0) {
			error++;
			tst_resm(TFAIL,
				 "thread %d (%s) terminated unsuccessfully %d "
				 "errors/%d loops\n%s",
				 th_num, pcom->th_func.fident, pcom->th_nerror,
				 pcom->th_nloop, pcom->detail_data);
		} else if (debug) {
			tst_resm(TINFO,
				 "thread %d (%s) terminated successfully %d loops",
				 th_num, pcom->th_func.fident,
				 pcom->th_nloop - 1);
		}
		SAFE_FREE(pcom);

	}
	SAFE_FREE(tabcom);
	SAFE_FREE(threads);
	tst_rmdir();
	if (error)
		exit(1);
	else
		exit(0);
	return 0;
}

/*----------------------------------------------------------------------+
|			    handle_signals ()				|
| ======================================================================|
|									|
| Function:  ....							|
|	    If SIGALRM or SIGUSR1 or SIGINT : cancel threads		|
|									|
| Updates:   ....							|
|									|
+-----------------------------------------------------------------------*/
static void *handle_signals(void *arg)
{
	sigset_t signals_set;
	int thd;
	int sig;
	int retvalsig = 0;

	if (debug)
		tst_resm(TINFO, "signal handler %lu started", pthread_self());
	/*
	 * Set up the signals that we want to handle...
	 */
	sigemptyset(&signals_set);
	sigaddset(&signals_set, SIGINT);
	sigaddset(&signals_set, SIGQUIT);
	sigaddset(&signals_set, SIGTERM);
	sigaddset(&signals_set, SIGUSR1);
	sigaddset(&signals_set, SIGALRM);
	while (1) {
		if (debug)
			tst_resm(TINFO, "Signal handler starts waiting...");

		sigwait(&signals_set, &sig);
		if (debug)
			tst_resm(TINFO, "Signal handler caught signal %d", sig);

		switch (sig) {
		case SIGALRM:
		case SIGUSR1:
		case SIGINT:
			if (sig_cancel)
				tst_resm(TINFO,
					 "Signal handler: already finished; "
					 "ignoring signal");
			else {
				/*
				 * Have to signal all non started threads...
				 */

				retvalsig = pthread_mutex_lock(&sig_mutex);
				if (retvalsig != 0)
					sys_error
					    ("handle_signal : mutex_lock(&sig_mutex) FAILED",
					     __LINE__);

				sig_cancel = 1;
				retvalsig = pthread_mutex_unlock(&sig_mutex);
				if (retvalsig != 0)
					sys_error
					    ("handle_signal : mutex_unlock(&sig_mutex) FAILED",
					     __LINE__);

				/*
				 * ......... and all started
				 */
				for (thd = 0; thd < indice; thd++) {
					if (debug)
						tst_resm(TINFO,
							 "signal handler: "
							 "cancelling thread (%d of "
							 "%d)", thd, indice);
					retvalsig =
					    pthread_cancel(threads[thd]);
					if (retvalsig != 0)
						sys_error
						    ("handle_signal : cancel FAILED",
						     __LINE__);
				}
			}
			break;
		case SIGQUIT:
			tst_resm(TINFO,
				 "Signal handler: Caught SIGQUIT; doing nothing");
			break;
		case SIGTERM:
			tst_resm(TINFO,
				 "Signal handler: Caught SIGTERM; doing nothing");
			break;
		default:
			exit(1);
		}
	}
	return NULL;
}

/*----------------------------------------------------------------------+
 |				error ()				|
 | =====================================================================|
 |									|
 | Function:  Prints out message and exits...				|
 |									|
 +----------------------------------------------------------------------*/
static void error(const char *msg, int line)
{
	tst_brkm(TFAIL, cleanup, "ERROR [line: %d] %s", line, msg);
}

/*----------------------------------------------------------------------+
 |			     sys_error ()				|
 | =====================================================================|
 |									|
 | Function:  Creates system error message and calls error ()		|
 |									|
 +----------------------------------------------------------------------*/
/*
 * XXX (garrcoop): the way that this is being called is just plain wrong.
 * pthread(5) returns 0 or errnos, not necessarily sets errno to a sensible
 * value.
 */
static void sys_error(const char *msg, int line)
{
	char syserr_msg[256];

	sprintf(syserr_msg, "%s: %s", msg, strerror(errno));
	error(syserr_msg, line);
}
