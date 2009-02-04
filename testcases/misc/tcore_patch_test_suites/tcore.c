/*
 *
 *   Copyright (c) Min Guo <min.guo@intel.com>., 2003
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
 */
 

// Use gcc -o xmm xmm.c -pthread -lm to compile.
#include "test.h"
#include "usctest.h"

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */
/* Global Variables */
char *TCID     = "tcore";            /* test program identifier.              */

#if defined __i386__ || defined(__x86_64__)
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#define BUFFER_SIZE 16

int  TST_TOTAL = 1;                  /* total number of tests in this file.   */

/* Circular buffer of integers. */

struct prodcons
{
  int buffer[BUFFER_SIZE];	/* the actual data */
  pthread_mutex_t lock;		/* mutex ensuring exclusive access to buffer */
  int readpos, writepos;	/* positions for reading and writing */
  pthread_cond_t notempty;	/* signaled when buffer is not empty */
  pthread_cond_t notfull;	/* signaled when buffer is not full */
};

/* Initialize a buffer */
static void
init (struct prodcons *b)
{
  pthread_mutex_init (&b->lock, NULL);
  pthread_cond_init (&b->notempty, NULL);
  pthread_cond_init (&b->notfull, NULL);
  b->readpos = 0;
  b->writepos = 0;
}

/* Store an integer in the buffer */
static void
put (struct prodcons *b, int data)
{
  pthread_mutex_lock (&b->lock);
  /* Wait until buffer is not full */
  while ((b->writepos + 1) % BUFFER_SIZE == b->readpos)
    {
      pthread_cond_wait (&b->notfull, &b->lock);
      /* pthread_cond_wait reacquired b->lock before returning */
    }
  /* Write the data and advance write pointer */
  b->buffer[b->writepos] = data;
  b->writepos++;
  if (b->writepos >= BUFFER_SIZE)
    b->writepos = 0;
  /* Signal that the buffer is now not empty */
  pthread_cond_signal (&b->notempty);
  pthread_mutex_unlock (&b->lock);
}

/* Read and remove an integer from the buffer */
static int
get (struct prodcons *b)
{
  int data;
  pthread_mutex_lock (&b->lock);
  /* Wait until buffer is not empty */
  while (b->writepos == b->readpos)
    {
      pthread_cond_wait (&b->notempty, &b->lock);
    }
  /* Read the data and advance read pointer */
  data = b->buffer[b->readpos];
  b->readpos++;
  if (b->readpos >= BUFFER_SIZE)
    b->readpos = 0;
  /* Signal that the buffer is now not full */
  pthread_cond_signal (&b->notfull);
  pthread_mutex_unlock (&b->lock);
  return data;
}

/* A test program: one thread inserts integers from 1 to 10000,
   the other reads them and prints them. */

#define OVER (-1)

struct prodcons buffer;

static void *
producer (void *data)
{
  int n;
  pid_t pid;
  long double a3 = 100.5678943, b3 = 200.578435698;
  long double c3, d3, e3, f3;
  a3 += b3;
  a3 *= pow(b3, 2);
  pid = getpid();
  tst_resm(TINFO,"producer pid=%d", pid);
  sleep(1);
  for (n = 0; n < 10000; n++)
    {
      tst_resm(TINFO,"%d --->", n);
      put (&buffer, n);

      if (n==7686) {
	    system("ps ax | grep ex");
	   c3 = pow(pid, pid);
	   d3 = log(pid);
	   e3 = c3 * d3;
	   f3 = c3 / d3;
 	{
	char buf[16];
	sprintf(buf, "%d%d\n", pid, pid);
        asm volatile ("movups (%0), %%xmm1;"::"r" (buf):"memory");
	}
	    sleep(1);
      }
    }
  put (&buffer, OVER);
  return NULL;
}

static void *
consumer (void *data)
{
  int d;
  char *junk = NULL;
  pid_t pid;
  long double a2 = 10002.5, b2 = 2888883.5;
  long double d2, e2, f2;
  a2 += b2;
  pid = getpid();
  tst_resm(TINFO,"consumer pid=%d", pid);
  sleep(1);
  while (1)
    {
      d = get (&buffer);
      if (d == OVER)
	break;
      tst_resm(TINFO,"---> %d", d);
      if (d==7688) {
	    system("ps ax | grep ex");
	    d2 = pid * a2 / b2;
	    e2 = tan(pid);
	    f2 = cos (pid)/d2;
 	{
	char buf[16];
	char buf1[16];
	sprintf(buf, "%d%d\n", pid, pid);
	sprintf(buf1,"%Lf",d2);
        asm volatile ("movups (%0), %%xmm2;":: "r" (buf):"memory");
        asm volatile ("movups (%0), %%xmm5;":: "r" (buf):"memory");
	}
	    *junk = 0;
      }
    }
  return NULL;
}

int
main (void)
{
  pthread_t th_a, th_b;
  void *retval;
  double a1 = 1.5, b1 = 2.5;
  long double c1 ;
  pid_t pid;
  a1 += b1;

  pid = getpid();

  init (&buffer);
  /* Create the threads */
  pthread_create (&th_a, NULL, producer, 0);
  pthread_create (&th_b, NULL, consumer, 0);
  
  c1 = exp(pid);
  /* Wait until producer and consumer finish. */
  pthread_join (th_a, &retval);
  pthread_join (th_b, &retval);
  return 0;
}

#else /* Not __i386__ */


int TST_TOTAL = 0;              /* Total number of test cases. */

int main() {
        tst_resm(TCONF, "tcore only works on x86 systems ...");
        tst_exit();
}

#endif /* __i386__ */
