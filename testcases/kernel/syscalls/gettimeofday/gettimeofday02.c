/* Check if gettimeofday is monotonous */

#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <test.h>
#include <usctest.h>

char *TCID="gettimeofday02"; 		/* Test program identifier.    */
int TST_TOTAL=1;    		/* Total number of test cases. */

int Tflag; 
char *tlen = "30"; 

sig_atomic_t done;

option_t opts[] = { { "T:", &Tflag, &tlen }, {} } ;

void breakout(int sig) 
{ 
	done = 1;
} 

void cleanup(void)
{
	
	TEST_CLEANUP;
	
	tst_exit();
}

void help()
{
	printf("  -T len  seconds to test gettimeofday (default %s)\n",tlen); 
}

int main(int ac, char **av) 
{
	struct timeval tv1,tv2;
	char *msg; 

	if ((msg = parse_opts(ac, av, opts, help)) != (char *)NULL) {
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);
	}
	
	tst_sig(NOFORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;
	
	tst_resm(TINFO, "checking if gettimeofday is monotonous, takes %ss\n",tlen); 
	signal(SIGALRM, breakout); 
	alarm(atoi(tlen));  

	gettimeofday(&tv1,NULL);
	while (!done) {
		gettimeofday(&tv2,NULL);
		if (	(tv2.tv_usec < tv1.tv_usec) &&
			(tv2.tv_sec <= tv1.tv_sec)
			) {
			tst_resm(TFAIL, "Time is going backwards (old %d.%d vs new %d.%d!",tv1.tv_sec,tv1.tv_usec,tv2.tv_sec,tv2.tv_usec);
			cleanup();
			exit(1);
		}
		tv1=tv2;
	}

	tst_resm(TPASS, "gettimeofday monotonous in %s seconds\n", tlen);

	cleanup();
	exit(0);
}
