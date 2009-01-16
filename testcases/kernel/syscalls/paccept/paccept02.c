/******************************************************************************/
/*                                                                            */
/* Copyright (c) Ulrich Drepper <drepper@redhat.com>                          */
/* Copyright (c) International Business Machines  Corp., 2009                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                  */
/* the GNU General Public License for more details.                           */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* File:        paccept02.c                                                   */
/*                                                                            */
/* Description: This Program tests the new system call introduced in 2.6.27.  */
/*              Ulrich´s comment as in:                                       */
/* http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=77d2720059618b9b6e827a8b73831eb6c6fad63c */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/* paccept02 [-c n] [-e][-i n] [-I x] [-p x] [-t]                      */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   paccept02                                                     */
/*                                                                            */
/* Author:      Ulrich Drepper <drepper@redhat.com>                           */
/*                                                                            */
/* History:     Created - Jan 13 2009 - Ulrich Drepper <drepper@redhat.com>   */
/*              Ported to LTP                                                 */
/*                      - Jan 13 2009 - Subrata <subrata@linux.vnet.ibm.com>  */
/******************************************************************************/
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/syscall.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

#ifndef __NR_paccept
# ifdef __x86_64__
#  define __NR_paccept 288
# elif defined __i386__
#  define SYS_PACCEPT 18
#  define USE_SOCKETCALL 1
# else
#  error "need __NR_paccept"
# endif
#endif

#ifdef USE_SOCKETCALL
# define paccept(fd, addr, addrlen, mask, flags) \
  ({ long args[6] = { \
       (long) fd, (long) addr, (long) addrlen, (long) mask, 8, (long) flags }; \
     syscall (__NR_socketcall, SYS_PACCEPT, args); })
#else
# define paccept(fd, addr, addrlen, mask, flags) \
  syscall (__NR_paccept, fd, addr, addrlen, mask, 8, flags)
#endif

#define PORT 57392

#define SOCK_NONBLOCK O_NONBLOCK

static pthread_barrier_t b;

static void * tf (void *arg) {
  pthread_barrier_wait (&b);
  int s = socket (AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  sin.sin_port = htons (PORT);
  connect (s, (const struct sockaddr *) &sin, sizeof (sin));
  close (s);
  pthread_barrier_wait (&b);

  pthread_barrier_wait (&b);
  s = socket (AF_INET, SOCK_STREAM, 0);
  sin.sin_port = htons (PORT);
  connect (s, (const struct sockaddr *) &sin, sizeof (sin));
  close (s);
  pthread_barrier_wait (&b);
  return NULL;
}

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "paccept02"; /* test program identifier.              */
int  testno;
int  TST_TOTAL = 1;                  /* total number of tests in this file.   */

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
extern void cleanup() {
  /* Remove tmp dir and all files in it */
  TEST_CLEANUP;
  tst_rmdir();

  /* Exit with appropriate return code. */
  tst_exit();
}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*              On success - returns 0.                                       */
/*                                                                            */
/******************************************************************************/
void setup() {
  /* Capture signals if any */
  /* Create temporary directories */
  TEST_PAUSE;
  tst_tmpdir();
}

int main (int argc, char *argv[]) {
  int s, fl, s2, reuse;
  struct sockaddr_in sin;
  pthread_t th;
  int lc;                 /* loop counter */
  char *msg;              /* message returned from parse_opts */

  /* Parse standard options given to run the test. */
  msg = parse_opts(argc, argv, (option_t *) NULL, NULL);
  if (msg != (char *) NULL) {
      tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
      tst_exit();
  }
  if((tst_kvercmp(2, 6, 27)) < 0) {
                tst_resm(TCONF, "This test can only run on kernels that are 2.6.27 and higher");
                tst_exit();
  }
  setup();

  /* Check looping state if -i option given */
  for (lc = 0; TEST_LOOPING(lc); ++lc) {
       Tst_count = 0;
       for (testno=0; testno < TST_TOTAL; ++testno) {
            pthread_barrier_init (&b, NULL, 2);
            if (pthread_create (&th, NULL, tf, NULL) != 0) {
                tst_brkm(TBROK, cleanup, "pthread_create failed");
                tst_exit();
            }

            s = socket (AF_INET, SOCK_STREAM, 0);
            reuse = 1;
            setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));
            sin.sin_family = AF_INET;
            sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
            sin.sin_port = htons (PORT);
            bind (s, (struct sockaddr *) &sin, sizeof (sin));
            listen (s, SOMAXCONN);

            pthread_barrier_wait (&b);

            s2 = paccept (s, NULL, 0, NULL, 0);
            if (s2 < 0) {
                tst_resm(TFAIL, "paccept(0) failed");
                cleanup();
                tst_exit();
            }

            fl = fcntl (s2, F_GETFL);
            if (fl & O_NONBLOCK) {
                tst_resm(TFAIL, "paccept(0) set non-blocking mode");
                cleanup();
                tst_exit();
            }
            close (s2);
            close (s);

            pthread_barrier_wait (&b);

            s = socket (AF_INET, SOCK_STREAM, 0);
            sin.sin_port = htons (PORT);
            setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));
            bind (s, (struct sockaddr *) &sin, sizeof (sin));
            listen (s, SOMAXCONN);

            pthread_barrier_wait (&b);

            s2 = paccept (s, NULL, 0, NULL, SOCK_NONBLOCK);
            if (s2 < 0) {
                tst_resm(TFAIL, "paccept(SOCK_NONBLOCK) failed");
                cleanup();
                tst_exit();
            }

            fl = fcntl (s2, F_GETFL);
            if ((fl & O_NONBLOCK) == 0) {
                 tst_resm(TFAIL, "paccept(SOCK_NONBLOCK) does not set non-blocking mode");
                 cleanup();
                 tst_exit();
            }
            close (s2);
            close (s);
            pthread_barrier_wait (&b);
            tst_resm(TPASS, "paccept(SOCK_NONBLOCK) PASSED");
            cleanup();
       }
  }
  tst_exit();
}
