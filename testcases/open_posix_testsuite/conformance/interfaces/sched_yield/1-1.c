/* 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *
 * Test that the running thread relinquish the processor until it again becomes
 * the head of its thread list.
 *
 * Steps:
 *  1. Set the policy to SCHED_FIFO.
 *  2. Launch as many processes as number CPU minus 1. These processses set
 *     their priority to the max and block all but 1 processor.
 *  3. Launch a thread which increase a counter in a infinite loop.
 *  4. Launch a thread which call sched_yield() and check that the counter has
 *     changed since the call.
 */
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "posixtest.h"

#ifdef BSD
# include <sys/types.h>
# include <sys/param.h>
# include <sys/sysctl.h>
#endif

#ifdef HPUX
# include <sys/param.h>
# include <sys/pstat.h>
#endif


#define LOOP 10      /* Shall be >= 1 */


int nb_call = 0;

/* Get the number of CPUs */
int get_ncpu() {
	int ncpu = -1;

	/* This syscall is not POSIX but it should work on many system */
#ifdef _SC_NPROCESSORS_ONLN
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
#else
# ifdef BSD
	int mib[2];
	size_t len = sizeof(ncpu);
	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	sysctl(mib, 2, &ncpu, &len, NULL, 0);
# else
#  ifdef HPUX
	struct pst_dynamic psd; 
	pstat_getdynamic(&psd, sizeof(psd), 1, 0);
	ncpu = (int)psd.psd_proc_cnt; 
#  endif /* HPUX */
# endif /* BSD */
#endif /* _SC_NPROCESSORS_ONLN */  

	return ncpu;
}

void * runner(void * arg) {
	int i=0, nc, result = 0;
	
	for(;i<LOOP;i++){
		nc = nb_call;
		sched_yield();

		/* If the value of nb_call has not change since the last call
		   of sched_yield, that means that the thread does not 
		   relinquish the processor */
		if(nc == nb_call) {
			result++;
		}
	}

	pthread_exit((void*)(&result));
	
	return NULL;
}

void * busy_thread(){
	while(1){ 
		nb_call++;
		sched_yield();
	}

	return NULL;
}


void buzy_process(){
        struct sched_param param;

        param.sched_priority = sched_get_priority_max(SCHED_FIFO);
        if(sched_setscheduler(getpid(), SCHED_FIFO, &param) != 0) {
                perror("An error occurs when calling sched_setparam()");
                return;
        }

        /* to avoid blocking */
        alarm(2);
        while(1);

}


int main() {
	int i, ncpu;
	int *child_pid; 
	pthread_t tid, tid_runner;
	int *tmpresult, result;
	pthread_attr_t attr;
        struct sched_param param;


	ncpu = get_ncpu();
	if(ncpu == -1) {
		printf("Can not get the number of CPUs of your machines.\n");
		return PTS_UNRESOLVED;
	}
	
	/*if(ncpu > 1) {
		printf("Not tested on multi-processors machines.\n");
		return PTS_UNTESTED;
	}*/

        param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
        if(sched_setscheduler(getpid(), SCHED_FIFO, &param) != 0) {
		if(errno == EPERM){
			printf("This process does not have the permission to set its own scheduling policy.\nTry to launch this test as root.\n");
			return PTS_UNRESOLVED;
		}
		perror("An error occurs when calling sched_setscheduler()");
                return PTS_UNRESOLVED;
        }

	child_pid = malloc((ncpu-1)*sizeof(int));

	for(i=0; i<ncpu-1; i++) {
		child_pid[i] = fork();
		if(child_pid[i] == -1){
			perror("An error occurs when calling fork()");
			return PTS_UNRESOLVED;
		} else if (child_pid[i] == 0){
			
			buzy_process();

			printf("This code should not be executed.\n");
			return PTS_UNRESOLVED;
		}
	}


	pthread_attr_init(&attr);

	if(pthread_create(&tid, &attr, busy_thread, NULL) != 0) {
		perror("An error occurs when calling pthread_create()");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_create(&tid_runner, &attr, runner, NULL) != 0) {
		perror("An error occurs when calling pthread_create()");
		return PTS_UNRESOLVED;
	}

	if(pthread_join(tid_runner, (void**) &tmpresult) != 0) {
		perror("An error occurs when calling pthread_join()");
		return PTS_UNRESOLVED;
	}
	result = *tmpresult;

	if(result){
		printf("A thread does not relinquish the processor.\n");
		return PTS_FAIL;
	}
		
	printf("Test PASSED\n");
	return PTS_PASS;
			
}
