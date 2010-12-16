/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2007                 */
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
/* File:        support_numa.c                                                     */
/*                                                                            */
/* Description: Allocates 1MB of memory and touches it to verify numa         */
/*                                                                            */
/* Author:      Sivakumar Chinnaiah  Sivakumar.C@in.ibm.com                   */
/*                                                                            */
/* History:     Created - Jul 18 2007 - Sivakumar Chinnaiah                   */
/*                                                 Sivakumar.C@in.ibm.com     */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

/* Global Variables */
#define MB (1<<20)
#define PAGE_SIZE getpagesize()
#define barrier() __asm__ __volatile__("": : :"memory")

/* Extern Global Variables */
extern int  Tst_count;               /* to avoid compilation errors. */
extern char *TESTDIR;                /* to avoid compilation errors. */

/* Global Variables */
char *TCID     = "support_numa"; /* to avoid compilation errors. */
int  TST_TOTAL = 1;                  /* to avoid compilation errors. */

void sigfunc(int sig)
{
        tst_resm(TINFO, "#Caught signal signum=%d", sig);
}

/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Description: Alloctes 1MB of memory and touches it to verify numa behaviour*/
/*                                                                            */
/* Input:       Describe input arguments to this program                      */
/*               argv[1] ==1 then print pagesize                              */
/*               argv[1] ==2 then allocate 1MB of memory                      */
/*		 argv[1] ==3 then pause the program to catch sigint	      */
/*                                                                            */
/* Exit:       On failure - Exits with non-zero value.                        */
/*             On success - exits with 0 exit value.                          */
/*                                                                            */
/******************************************************************************/

int main( int argc, char *argv[] )
{
	int i;
	char *buf = NULL;
	int count=0;
	struct sigaction sa;

        switch(atoi(argv[1]))
	{
	case 1: printf("%d", PAGE_SIZE);
		tst_exit();
	case 2:
		buf = (char*) malloc(MB);
               	if (!buf)
		{
			tst_resm(TINFO, "#Memory is not available\n");
			tst_exit();
			exit(2);
		}
       		for (i=0; i<MB; i+= PAGE_SIZE)
		{
			count++;
               		buf[i] = 'a';
               		barrier();
       		}
		free(buf);
		tst_exit();
	case 3:
                /* Trap SIGINT */
                sa.sa_handler = sigfunc;
                sa.sa_flags = SA_RESTART;
                sigemptyset(&sa.sa_mask);
                if (sigaction(SIGINT, &sa, 0) < 0)
		{
			tst_brkm(TBROK, NULL, "#Sigaction SIGINT failed\n");
			tst_exit();
			exit(1);
		}
                /* wait for signat Int */
                pause();
		tst_exit();
	default:
		exit(1);
	}
}