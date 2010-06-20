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

#ifdef __linux__ 
#define _GNU_SOURCE
#endif

#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define LOOP 1000     /* Shall be >= 1 */

volatile int nb_call = 0;

/* Get the number of CPUs */
int get_ncpu() {
	int ncpu = -1;

	/* This syscall is not POSIX but it should work on many system */
#ifdef _SC_NPROCESSORS_ONLN
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	return ncpu;
}

#ifdef __linux__
int set_process_affinity(int cpu)
{
	int retval = -1;
	cpu_set_t cpu_mask;
	
	CPU_ZERO(&cpu_mask);
	if (cpu >= 0 && cpu <= CPU_SETSIZE) {
		CPU_SET(cpu, &cpu_mask);
	} else {
		fprintf (stderr, "Wrong cpu id: %d\n", cpu); 
		return -1;
	}
		
//#ifndef P2_SCHED_SETAFFINITY
	retval = sched_setaffinity(0, sizeof(cpu_mask), &cpu_mask);
//#else
//	retval = sched_setaffinity(0, &cpu_mask);
//#endif
	if (retval == -1)
	perror("Error at sched_setaffinity()");
        
        return retval;
}

int set_thread_affinity(int cpu)
{
	int retval = -1;
	cpu_set_t cpu_mask;
	
	CPU_ZERO(&cpu_mask);
	if (cpu >= 0 && cpu <= CPU_SETSIZE) {
		CPU_SET(cpu, &cpu_mask);
	} else {
		fprintf (stderr, "Wrong cpu id: %d\n", cpu); 
		return -1;
	}
//#ifndef P2_PTHREAD_SETAFFINITY
	retval = pthread_setaffinity_np(pthread_self(), 
			sizeof(cpu_mask), &cpu_mask);
//#else
//	retval = pthread_setaffinity_np(pthread_self(), &cpu_mask);
//#endif
        if (retval != 0)
	fprintf (stderr, "Error at pthread_setaffinity_np():\n");
	return retval;
}

#endif
        
void * runner(void * arg) {
	int i=0, nc;
	long result = 0;
#ifdef __linux__       
        set_thread_affinity(*(int *)arg);
        fprintf(stderr, "%ld bind to cpu: %d\n", pthread_self(), *(int*)arg);
#endif
	
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
        
	pthread_exit((void*)(result));
	
	return NULL;
}

void * busy_thread(void *arg){
#ifdef __linux__
        set_thread_affinity(*(int *)arg);
        fprintf(stderr, "%ld bind to cpu: %d\n", pthread_self(), *(int*)arg);
#endif
        while(1){ 
                nb_call++;
		sched_yield();
	}

	return NULL;
}


void busy_process(int cpu){
        struct sched_param param;

#ifdef __linux__        
        /* Bind to a processor */
        set_process_affinity(cpu);
        fprintf(stderr, "%d bind to cpu: %d\n", getpid(), cpu);
#endif
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
	void *tmpresult;
	long result;
	pthread_attr_t attr;
        struct sched_param param;
        int thread_cpu;


	ncpu = get_ncpu();
	if(ncpu == -1) {
		printf("Can not get the number of CPUs of your machines.\n");
		return PTS_UNRESOLVED;
	}
	
	printf("System has %d processors\n", ncpu);

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
			
			busy_process(i);

			printf("This code should not be executed.\n");
			return PTS_UNRESOLVED;
		}
	}


	pthread_attr_init(&attr);
        pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

        thread_cpu = ncpu -1;
	if(pthread_create(&tid, &attr, busy_thread, &thread_cpu) != 0) {
		perror("An error occurs when calling pthread_create()");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_create(&tid_runner, &attr, runner, &thread_cpu) != 0) {
		perror("An error occurs when calling pthread_create()");
		return PTS_UNRESOLVED;
	}

	if(pthread_join(tid_runner, &tmpresult) != 0) {
		perror("An error occurs when calling pthread_join()");
		return PTS_UNRESOLVED;
	}
	
        for(i=0; i<ncpu-1; i++)
                waitpid(child_pid[i], NULL, 0);
	
        result = (long)tmpresult;
	if(result){
		printf("A thread does not relinquish the processor.\n");
		return PTS_FAIL;
	}
        
	printf("Test PASSED\n");
	return PTS_PASS;
			
}
