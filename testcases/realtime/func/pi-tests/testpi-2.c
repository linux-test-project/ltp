/******************************************************************************
 *
 *   Copyright © International Business Machines  Corp., 2005, 2008
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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * NAME
 *      testpi-2.c
 *
 * DESCRIPTION
 *
 *
 *
 * USAGE:
 *      Use run_auto.sh script in current directory to build and run test.
 *
 * AUTHOR
 *
 *
 * HISTORY
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <librttest.h>

pthread_barrier_t barrier;

void usage(void)
{
        rt_help();
        printf("testpi-2 specific options:\n");
}

int parse_args(int c, char *v)
{

        int handled = 1;
        switch (c) {
                case 'h':
                        usage();
                        exit(0);
                default:
                        handled = 0;
                        break;
        }
        return handled;
}

int gettid(void)
{
        return syscall(__NR_gettid);
}

typedef void* (*entrypoint_t)(void*);

#define THREAD_STOP    1

pthread_mutex_t glob_mutex;

void* func_lowrt(void* arg)
{
  struct thread*  pthr = (struct thread* )arg ;
  int rc, i, j, tid = gettid();
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);

  rc = sched_setaffinity(0, sizeof(mask), &mask);
  if (rc < 0) {
     printf("Thread %d: Can't set affinity: %d %s\n", tid, rc, strerror(rc));
     exit(-1);
  }

  printf("Thread %d started running with priority %d\n", tid, pthr->priority);
  pthread_mutex_lock(&glob_mutex);
  printf("Thread %d at start pthread pol %d pri %d - Got global lock\n", tid, pthr->policy, pthr->priority);
  /* Wait for other RT threads to start up */
  pthread_barrier_wait(&barrier);

  for (i=0;i<10000;i++) {
    if (i%100 == 0) {
      printf("Thread %d loop %d pthread pol %d pri %d\n", tid, i, pthr->policy, pthr->priority);
      fflush(NULL);
    }
    pthr->id++;
    for (j=0;j<5000;j++) {
      pthread_mutex_lock(&(pthr->mutex));
      pthread_mutex_unlock(&(pthr->mutex));
    }
  }
  pthread_mutex_unlock(&glob_mutex);
  return NULL;
}

void* func_rt(void* arg)
{
  struct thread*  pthr = (struct thread* )arg;
  int rc, i, j, tid = gettid();
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);

  rc = sched_setaffinity(0, sizeof(mask), &mask);
  if (rc < 0) {
     printf("Thread %d: Can't set affinity: %d %s\n", tid, rc, strerror(rc));
     exit(-1);
  }

  printf("Thread %d started running with prio %d\n", tid, pthr->priority);
  pthread_barrier_wait(&barrier);
  pthread_mutex_lock(&glob_mutex);
  printf("Thread %d at start pthread pol %d pri %d - Got global lock\n", tid, pthr->policy, pthr->priority);

  /* we just use the mutex as something to slow things down */
  /* say who we are and then do nothing for a while.  The aim
   * of this is to show that high priority threads make more
   * progress than lower priority threads..
   */
  for (i=0;i<1000;i++) {
    if (i%100 == 0) {
      printf("Thread %d loop %d pthread pol %d pri %d\n", tid, i, pthr->policy, pthr->priority);
      fflush(NULL);
    }
    pthr->id++;
    for (j=0;j<5000;j++) {
      pthread_mutex_lock(&(pthr->mutex));
      pthread_mutex_unlock(&(pthr->mutex));
    }
  }
  pthread_mutex_unlock(&glob_mutex);
  return NULL;
}

void* func_noise(void* arg)
{
  struct thread*  pthr = (struct thread* )arg;
  int rc, i, j, tid = gettid();
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);

  rc = sched_setaffinity(0, sizeof(mask), &mask);
  if (rc < 0) {
     printf("Thread %d: Can't set affinity: %d %s\n", tid, rc, strerror(rc));
     exit(-1);
  }

  printf("Noise Thread %d started running with prio %d\n", tid, pthr->priority);
  pthread_barrier_wait(&barrier);

  for (i=0;i<10000;i++) {
    if (i%100 == 0) {
      printf("Noise Thread %d loop %d pthread pol %d pri %d\n", tid, i, pthr->policy, pthr->priority);
      fflush(NULL);
    }
    pthr->id++;
    for (j=0;j<5000;j++) {
      pthread_mutex_lock(&(pthr->mutex));
      pthread_mutex_unlock(&(pthr->mutex));
    }
  }
  return NULL;
}

/*
 * Test pthread creation at different thread priorities.
 */
int main(int argc, char* argv[]) {
  pthread_mutexattr_t mutexattr;
  int i, retc, protocol, nopi = 0;
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  setup();
  rt_init("h",parse_args,argc,argv);

  if ((retc = pthread_barrier_init(&barrier, NULL, 5))) {
    printf("pthread_barrier_init failed: %s\n", strerror(retc));
    exit(retc);
  }

  retc = sched_setaffinity(0, sizeof(mask), &mask);
  if (retc < 0) {
     printf("Main Thread: Can't set affinity: %d %s\n", retc, strerror(retc));
     exit(-1);
  }

  for (i=0;i<argc;i++) {
    if (strcmp(argv[i],"nopi") == 0) nopi = 1;
  }

  printf("Start %s\n",argv[0]);

  if (!nopi) {
    if (pthread_mutexattr_init(&mutexattr) != 0) {
      printf("Failed to init mutexattr\n");
    };
    if (pthread_mutexattr_setprotocol(&mutexattr, PTHREAD_PRIO_INHERIT) != 0) {
      printf("Can't set protocol prio inherit\n");
    }
    if (pthread_mutexattr_getprotocol(&mutexattr, &protocol) != 0) {
      printf("Can't get mutexattr protocol\n");
    } else {
      printf("protocol in mutexattr is %d\n", protocol);
    }
    if ((retc = pthread_mutex_init(&glob_mutex, &mutexattr)) != 0) {
      printf("Failed to init mutex: %d\n", retc);
    }
  }

  create_rr_thread(func_lowrt, NULL, 10);
  create_rr_thread(func_rt, NULL, 20);
  create_fifo_thread(func_rt, NULL, 30);
  create_fifo_thread(func_rt, NULL, 40);
  create_rr_thread(func_noise, NULL, 40);

  printf("Joining threads\n");
  join_threads();
  printf("Done\n");
  printf("Criteria: Low Priority Thread and High Priority Thread should prempt each other multiple times\n");
  return 0;
}
