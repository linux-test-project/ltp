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
/* File:        paccept01.c                                                   */
/*                                                                            */
/* Description: This Program tests the new system call introduced in 2.6.27.  */
/*              Ulrich´s comment as in:                                       */
/* http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=commit;h=aaca0bdca573f3f51ea03139f9c7289541e7bca3 */
/*              says:                                                         */
/*              flag parameters: paccept. This patch is by far the most       */
/*              complex in the series.  It adds a new syscall paccept.  This  */
/*              syscall differs from accept in that it adds (at the userlevel)*/
/*              two additional parameters:                                    */
/*              - a signal mask                                               */
/*              - a flags value                                               */
/*                                                                            */
/*              The flags parameter can be used to set flag like SOCK_CLOEXEC.*/
/*              This is imlpemented here as well.  Some people argued that    */
/*              this is a property which should be inherited from the file    */
/*              desriptor for the server but this is against POSIX.Additionally*/
/*              ,we really want the signal mask parameter as well (similar to */
/*              pselect, ppoll, etc).  So an interface change in inevitable.  */
/*              The flag value is the same as for socket and socketpair.  I   */
/*              think diverging here will only create confusion.  Similar to  */
/*              the filesystem interfaces where the use of the O_* constants  */
/*              differs, it is acceptable here. The signal mask is handled as */
/*              for pselect etc.  The mask is temporarily installed for the   */
/*              thread and removed before the call returns.  I modeled the    */
/*              code after pselect.  If there is a problem it's likely also in*/
/*              pselect. For architectures which use socketcall I maintained  */
/*              this interface instead of adding a system call.  The symmetry */
/*              shouldn't be broken. The following test must be adjusted for  */
/*              architectures other than x86 and x86-64 and in case the       */
/*              syscall numbers changed.                                      */
/*                                                                            */
/* Usage:  <for command-line>                                                 */
/*  paccept01 [-c n] [-e][-i n] [-I x] [-p x] [-t]                             */
/*      where,  -c n : Run n copies concurrently.                             */
/*              -e   : Turn on errno logging.                                 */
/*              -i n : Execute test n times.                                  */
/*              -I x : Execute test for x seconds.                            */
/*              -P x : Pause for x seconds between iterations.                */
/*              -t   : Turn on syscall timing.                                */
/*                                                                            */
/* Total Tests: 1                                                             */
/*                                                                            */
/* Test Name:   paccept01                                                      */
/*                                                                            */
/* Author:      Ulrich Drepper <drepper@redhat.com>                           */
/*                                                                            */
/* History:     Created - Jan 06 2009 - Ulrich Drepper <drepper@redhat.com>   */
/*              Ported to LTP                                                 */
/*                      - Jan 06 2009 - Subrata <subrata@linux.vnet.ibm.com>  */
/******************************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/syscall.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"

#ifndef O_CLOEXEC
 #define O_CLOEXEC 02000000
#endif

#ifndef __NR_paccept
 #ifdef __x86_64__
  #define __NR_paccept 288
 #elif defined __i386__
  #define SYS_PACCEPT 18
  #define USE_SOCKETCALL 1
 #else
  #error "need __NR_paccept"
 #endif
#endif

#ifdef USE_SOCKETCALL
 #define paccept(fd, addr, addrlen, mask, flags) \
  ({ long args[6] = { \
       (long) fd, (long) addr, (long) addrlen, (long) mask, 8, (long) flags }; \
     syscall (__NR_socketcall, SYS_PACCEPT, args); })
#else
 #define paccept(fd, addr, addrlen, mask, flags) \
  syscall (__NR_paccept, fd, addr, addrlen, mask, 8, flags)
#endif

#define PORT 57392
#define SOCK_CLOEXEC O_CLOEXEC

static pthread_barrier_t b;

/* Extern Global Variables */
extern int  Tst_count;               /* counter for tst_xxx routines.         */
extern char *TESTDIR;                /* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID     = "paccept01";        /* test program identifier.              */
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
  s = socket (AF_INET, SOCK_STREAM, 0);
  sin.sin_port = htons (PORT);
  connect (s, (const struct sockaddr *) &sin, sizeof (sin));
  close (s);
  pthread_barrier_wait (&b);

  pthread_barrier_wait (&b);
  sleep (2);
  pthread_kill ((pthread_t) arg, SIGUSR1);
  return NULL;
}

static void handler (int s) {
}

int main (int argc, char *argv[]) {
  int s,s2, coe, reuse;
  pthread_barrier_init (&b, NULL, 2);
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

  if (pthread_create (&th, NULL, tf, (void *) pthread_self ()) != 0) {
      tst_brkm(TBROK, cleanup, "pthread_create failed");
      tst_exit();
    }

  /* Check looping state if -i option given */
  for (lc = 0; TEST_LOOPING(lc); ++lc) {
       Tst_count = 0;
       for (testno=0; testno < TST_TOTAL; ++testno) {
            s = socket(AF_INET, SOCK_STREAM, 0);
            reuse = 1;
            setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));
            sin.sin_family = AF_INET;
            sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
            sin.sin_port = htons (PORT);
            bind(s, (struct sockaddr *) &sin, sizeof (sin));
            listen(s, SOMAXCONN);

            pthread_barrier_wait (&b);

            s2 = paccept(s, NULL, 0, NULL, 0);
            if (s2 < 0) {
                tst_resm(TFAIL, "paccept(0) failed");
                cleanup();
                tst_exit();
            }

            coe = fcntl(s2, F_GETFD);
            if (coe & FD_CLOEXEC) {
                tst_resm(TFAIL, "paccept(0) set close-on-exec-flag");
                cleanup();
                tst_exit();
            }
            close (s2);

            pthread_barrier_wait (&b);
            s2 = paccept(s, NULL, 0, NULL, SOCK_CLOEXEC);
            if (s2 < 0) {
                tst_resm(TFAIL, "paccept(SOCK_CLOEXEC) failed");
                cleanup();
                tst_exit();
            }

            coe = fcntl(s2, F_GETFD);
            if ((coe & FD_CLOEXEC) == 0) {
                 tst_resm(TFAIL, "paccept(SOCK_CLOEXEC) does not set close-on-exec flag");
                 cleanup();
                 tst_exit();
            }
            close(s2);

            pthread_barrier_wait (&b);

            struct sigaction sa;
            sa.sa_handler = handler;
            sa.sa_flags = 0;
            sigemptyset (&sa.sa_mask);
            sigaction (SIGUSR1, &sa, NULL);

            sigset_t ss;
            pthread_sigmask (SIG_SETMASK, NULL, &ss);
            sigaddset (&ss, SIGUSR1);
            pthread_sigmask (SIG_SETMASK, &ss, NULL);

            sigdelset (&ss, SIGUSR1);
            alarm (4);
            pthread_barrier_wait (&b);

            errno = 0 ;
            s2 = paccept(s, NULL, 0, &ss, 0);
            if (s2 != -1 || errno != EINTR) {
                tst_resm(TFAIL, "paccept did not fail with EINTR");
                cleanup();
                tst_exit();
            }
            close (s);
            tst_resm(TPASS, "paccept(SOCK_CLOEXEC) PASSED");
            cleanup();
           }
      }
 tst_exit();
}
