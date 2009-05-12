/******************************************************************************/
/*                                                                            */
/* Copyright (C) 2008, Linux Foundation,                                      */
/* written by Michael Kerrisk <mtk.manpages@gmail.com>                        */
/*                                                                            */
/* Licensed under the GNU GPLv2 or later.                                     */
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
/* File:        accept4_01.c                                                  */
/*                                                                            */
/* Description: This will test the newly introduced syscall accept4()         */
/*                                                                            */
/* Total Tests: 4                                                             */
/*                                                                            */
/* Test Name:   accept4_01                                                    */
/*                                                                            */
/* Author:      Michael Kerrisk <mtk.manpages@gmail.com>                      */
/*                                                                            */
/* History:     Created - Nov 17 2008 - Michael <mtk.manpages@gmail.com>      */
/*              Initial Porting to LTP                                        */
/*                      - Nov 17 2008 - Subrata <subrata@linux.vnet.ibm.com>  */
/*                                                                            */
/******************************************************************************/

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

#include "linux_syscall_numbers.h"

#define PORT_NUM 33333

#define die(msg) do { tst_brkm(TBROK, cleanup, msg ": %s", strerror(errno)); } while (0)

/**********************************************************************/

/* The following is what we need until glibc gets a wrapper for
  accept4() */

/* Flags for socket(), socketpair(), accept4() */
#ifndef O_CLOEXEC
# define O_CLOEXEC 02000000
#endif

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC    O_CLOEXEC
#endif
#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK   O_NONBLOCK
#endif

#if defined(SYS_ACCEPT4)	/* the socketcall() number */
#define USE_SOCKETCALL 1
#endif

/* Extern Global Variables */
extern int Tst_count;		/* counter for tst_xxx routines.         */
extern char *TESTDIR;		/* temporary dir created by tst_tmpdir() */

/* Global Variables */
char *TCID = "accept04_01";	/* test program identifier.          */
int TST_TOTAL = 1;		/* total number of tests in this file.   */

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
/* We would need this for many more reasons in future                          */
extern void cleanup()
{
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
void setup()
{
	/* Capture signals if any */
	/* Create temporary directories */
	TEST_PAUSE;
	tst_tmpdir();
}
#if !(__GLIBC_PREREQ(2,10))
static int
accept4_01(int fd, struct sockaddr *sockaddr, socklen_t *addrlen, int flags)
{
#ifdef DEBUG
	tst_resm(TINFO, "Calling accept4(): flags = %x", flags);
	if (flags != 0) {
		tst_resm(TINFO, " (");
		if (flags & SOCK_CLOEXEC)
			tst_resm(TINFO, "SOCK_CLOEXEC");
		if ((flags & SOCK_CLOEXEC) && (flags & SOCK_NONBLOCK))
			tst_resm(TINFO, " ");
		if (flags & SOCK_NONBLOCK)
			tst_resm(TINFO, "SOCK_NONBLOCK");
		tst_resm(TINFO, ")");
	}
	tst_resm(TINFO, "\n");
#endif

#if USE_SOCKETCALL
	long args[6];

	args[0] = fd;
	args[1] = (long)sockaddr;
	args[2] = (long)addrlen;
	args[3] = flags;

	return syscall(__NR_socketcall, SYS_ACCEPT4, args);
#else
	return syscall(__NR_accept4, fd, sockaddr, addrlen, flags);
#endif
}
#endif

/**********************************************************************/

static void
do_test(int lfd, struct sockaddr_in *conn_addr,
	int closeonexec_flag, int nonblock_flag)
{
	int connfd, acceptfd;
	int fdf, flf, fdf_pass, flf_pass;
	struct sockaddr_in claddr;
	socklen_t addrlen;

#ifdef DEBUG
	tst_resm(TINFO, "=======================================\n");
#endif

	connfd = socket(AF_INET, SOCK_STREAM, 0);
	if (connfd == -1)
		die("Socket Error");
	if (connect(connfd, (struct sockaddr *)conn_addr,
		    sizeof(struct sockaddr_in)) == -1)
		die("Connect Error");

	addrlen = sizeof(struct sockaddr_in);
	#if !(__GLIBC_PREREQ(2,10))
	acceptfd = accept4_01(lfd, (struct sockaddr *)&claddr, &addrlen,
                           closeonexec_flag | nonblock_flag);
	#else
	acceptfd = accept4(lfd, (struct sockaddr *)&claddr, &addrlen,
			   closeonexec_flag | nonblock_flag);
	#endif
	if (acceptfd == -1) {
		tst_resm(TCONF,
			 "syscall __NR_accept4 not supported on your arch");
	}

	fdf = fcntl(acceptfd, F_GETFD);
	if (fdf == -1)
		die("fcntl:F_GETFD");
	fdf_pass = ((fdf & FD_CLOEXEC) != 0) ==
	    ((closeonexec_flag & SOCK_CLOEXEC) != 0);
#ifdef DEBUG
	tst_resm(TINFO, "Close-on-exec flag is %sset (%s); ",
		 (fdf & FD_CLOEXEC) ? "" : "not ", fdf_pass ? "OK" : "failed");
#endif
	if (!fdf_pass)
		tst_resm(TFAIL,
			 "Close-on-exec flag mismatch, should be %x, actual %x",
			 fdf & FD_CLOEXEC, closeonexec_flag & SOCK_CLOEXEC);

	flf = fcntl(acceptfd, F_GETFL);
	if (flf == -1)
		die("fcntl:F_GETFD");
	flf_pass = ((flf & O_NONBLOCK) != 0) ==
	    ((nonblock_flag & SOCK_NONBLOCK) != 0);
#ifdef DEBUG
	tst_resm(TINFO, "nonblock flag is %sset (%s)\n",
		 (flf & O_NONBLOCK) ? "" : "not ", flf_pass ? "OK" : "failed");
#endif
	if (!flf_pass)
		tst_resm(TFAIL,
			 "nonblock flag mismatch, should be %x, actual %x",
			 fdf & O_NONBLOCK, nonblock_flag & SOCK_NONBLOCK);

	close(acceptfd);
	close(connfd);

	if (fdf_pass && flf_pass)
		tst_resm(TPASS, "Test passed");
}

static int create_listening_socket(int port_num)
{
	struct sockaddr_in svaddr;
	int lfd;
	int optval;

	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svaddr.sin_port = htons(port_num);

	lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
		die("Socket Error");

	optval = 1;
	if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval,
		       sizeof(optval)) == -1)
		die("Setsockopt Error");

	if (bind(lfd, (struct sockaddr *)&svaddr,
		 sizeof(struct sockaddr_in)) == -1)
		die("Bind Error");

	if (listen(lfd, 5) == -1)
		die("Listen Error");

	return lfd;
}

int main(int argc, char *argv[])
{
#if defined(__NR_accept4) || defined(__NR_socketcall)
	struct sockaddr_in conn_addr;
	int lfd;
	int port_num;

	setup();
	port_num = (argc > 1) ? atoi(argv[1]) : PORT_NUM;

	memset(&conn_addr, 0, sizeof(struct sockaddr_in));
	conn_addr.sin_family = AF_INET;
	conn_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	conn_addr.sin_port = htons(port_num);

	lfd = create_listening_socket(port_num);

	do_test(lfd, &conn_addr, 0, 0);
	do_test(lfd, &conn_addr, SOCK_CLOEXEC, 0);
	do_test(lfd, &conn_addr, 0, SOCK_NONBLOCK);
	do_test(lfd, &conn_addr, SOCK_CLOEXEC, SOCK_NONBLOCK);
	close(lfd);
#else
	tst_resm(TCONF, "syscall __NR_accept4 not supported on your arch");
#endif

	cleanup();
	tst_exit();
}
