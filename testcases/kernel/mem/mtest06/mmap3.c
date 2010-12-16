/******************************************************************************/
/*									      */
/* Copyright (c) International Business Machines  Corp., 2001		      */
/*									      */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.					      */
/*									      */
/* This program is distributed in the hope that it will be useful,	      */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	      */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See	              */
/* the GNU General Public License for more details.			      */
/*									      */
/* You should have received a copy of the GNU General Public License	      */
/* along with this program;  if not, write to the Free Software		      */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									      */
/******************************************************************************/

/******************************************************************************/
/*									      */
/* History:	July - 16 - 2001 Created by Manoj Iyer, IBM Austin TX.	      */
/*			         email:manjo@austin.ibm.com		      */
/*                                                                            */
/*              July - 24 - 2001 Modified. Added loop and pass arguments to   */
/*                               the thread function. Fixed usage function.   */
/*				 added option to MAP_PRIVATE or MAP_SHARED.   */
/*                                                                            */
/*              Aug  - 01 - 2001 Modified. Added include file signal.h        */
/*                               has defines required by signal handler.      */
/*                                                                            */
/*		Oct  - 25 - 2001 Modified. Fixed bug in main(). Pthread_join  */
/*				 sets the return value of the thread in thread*/
/*			         return_status parameter.                     */
/*		Nov - 09  - 2001 Modified. Removed compile errors             */
/*				 - incomplete comment in line 301             */
/*				 - missing argument to printf in pthread_join */
/*                                                                            */
/*              Apr  - 16 - 2003 Modified - replaced tempnam() use with       */
/*                               mkstemp(). -Robbie Williamson                */
/*                               email:robbiew@us.ibm.com                     */
/*                                                                            */
/*              May  - 12 - 2003 Modified - remove the really large files     */
/*                               when we are done with the test - Paul Larson */
/*                               email:plars@linuxtestproject.org             */
/* File:	mmap3.c							      */
/*			         					      */
/* Description: Test the LINUX memory manager. The program is aimed at        */
/*              stressing the memory manager by repeaded map/write/unmap      */
/*		of file/memory of random size (maximum 1GB) this is done by   */
/*		multiple processes.					      */
/*			         					      */
/*		Create a file of random size upto 1000 times 4096, map it,    */
/*		change the contents of the file and unmap it. This is repeated*/
/*		several times for the specified number of hours by a certain. */
/*		number of processes.					      */
/*			         					      */
/******************************************************************************/

/* Include Files							      */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sched.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

/* Defines								      */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define prtln() printf(" I AM HERE ==> %s %d\n", __FILE__, __LINE__);

/******************************************************************************/
/*                                                                            */
/* Function:    mkfile                                                        */
/*                                                                            */
/* Description: Create a temparory file of ramdom size. 		      */
/*                                                                            */
/* Input:	NONE							      */
/*                                                                            */
/* Output:      size - size of the temp file  created 			      */
/*                                                                            */
/* Return:      int fd - file descriptor if the file was created.             */
/*              -1     - if it failed to create.                              */
/*                                                                            */
/******************************************************************************/
static int
mkfile(int  *size) 		/* size of the file to be generated in GB     */
{
    int   fd;			/* file descriptior of tmpfile		      */
    int   index = 0;		/* index into the file, fill it with a's      */
    char  buff[4096];		/* buffer that will be written to the file.   */
    char template[PATH_MAX];    /* template for temp file name                */

    /* fill the buffer with a's and open a temp file */
    memset(buff, 'a', 4096);
    snprintf(template, PATH_MAX, "ashfileXXXXXX");
    if ((fd = mkstemp(template)) == -1)
    {
        perror("mkfile(): mkstemp()");
	return -1;
    }
    unlink(template);

    srand(time(NULL)%100);
    *size = (1 + (int)(1000.0*rand()/(RAND_MAX+1.0))) * 4096;

    /* fill the file with the character a */
    while (index < *size)
    {
        index += 4096;
        if (write(fd, buff, 4096) == -1)
	{
	    perror("mkfile(): write()");
	    return -1;
	}
    }

    /* make sure a's are written to the file. */
    if (fsync(fd) == -1)
    {
	perror("mkfile(): fsync()");
	return -1;
    }
    return fd;
}

/******************************************************************************/
/*                                                                            */
/* Function:    sig_handler                                                   */
/*                                                                            */
/* Description: handle SIGALRM raised by set_timer(), SIGALRM is raised when  */
/*              the timer expires. If any other signal is recived exit the    */
/*              test.                                                         */
/*                                                                            *//* Input:       signal - signal number, intrested in SIGALRM!                 */
/*                                                                            */
/* Return:      exit 1 if unexpected signal is recived                        */
/*              exit 0 if SIGALRM is recieved                                 */
/*                                                                            */
/******************************************************************************/
static void
sig_handler(int signal)         /* signal number, set to handle SIGALRM       */{
    if (signal != SIGALRM)
    {
        fprintf(stderr, "sig_handlder(): unexpected signal caught [%d]\n",
            signal);
        exit(-1);
    }
    else
        fprintf(stdout, "Test ended, success\n");
    exit(0);
}

/******************************************************************************/
/*                                                                            */
/* Function:    set_timer                                                     */
/*                                                                            */
/* Description: set up a timer to user specified number of hours. SIGALRM is  */
/*              raised when the timer expires.                                */
/*                                                                            */
/* Input:       run_time - number of hours the test is intended to run.       */
/*                                                                            */
/* Return:      NONE                                                          */
/*                                                                            */
/******************************************************************************/
static void
set_timer(float run_time)         /* period for which test is intended to run   */{
    struct itimerval timer;     /* timer structure, tv_sec is set to run_time */
    memset(&timer, 0, sizeof(struct itimerval));
    timer.it_interval.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_value.tv_sec = (time_t)(run_time * 3600.0);

    if (setitimer(ITIMER_REAL, &timer, NULL))
    {
        perror("set_timer(): setitimer()");
        exit(1);
    }
}

/******************************************************************************//*								 	      */
/* Function:	usage							      */
/*									      */
/* Description:	Print the usage message.				      */
/*									      */
/* Return:	exits with -1						      */
/*									      */
/******************************************************************************/
static void
usage(char *progname)           /* name of this program                       */{
    fprintf(stderr,
               "Usage: %s -h -l -n -p -x\n"
               "\t -h help, usage message.\n"
               "\t -l number of map - write - unmap.    default: 1000\n"
               "\t -n number of LWP's to create.        default: 20\n"
               "\t -p set mapping to MAP_PRIVATE.       default: MAP_SHARED\n"
               "\t -x time for which test is to be run. default: 24 Hrs\n",
                    progname);
    exit(-1);
}

/******************************************************************************/
/*									      */
/* Function:	map_write_unmap						      */
/*									      */
/* Description:	map a file write a character to it and unmap, this is done for*/
/*		user defined number of times.				      */
/*									      */
/* Input:	arg[0]		   number of times map - write -unmap is done */
/* 		arg[1]		   Map type:  				      */
/*					TRUE  - MAP_PRIVATE 		      */
/*					FALSE - MAP_SHARED		      */
/*									      */
/* Return:	MWU_FAIL on error.				              */
/*              MWU_SUCCESS on error less completion of the loop.             */
/*									      */
/******************************************************************************/
void *
map_write_unmap(void *args)	/* file descriptor of the file to be mapped.  */
{
    int	    fsize;		/* size of the file to be created.	      */
    int     fd;			/* temporary file descriptor		      */
    int     mwu_ndx = 0;	/* index to number of map/write/unmap         */
    caddr_t *map_address;	/* pointer to file in memory		      */
    int     map_type;		/* MAP_PRIVATE or MAP_SHARED	              */
    long    *mwuargs =          /* local pointer to arguments		      */
		       (long *)args;

    while (mwu_ndx++ < (int)mwuargs[0])
    {
        if ((fd = mkfile(&fsize)) == -1)
        {
            fprintf(stderr,
            	"main(): mkfile(): Failed to create temp file.\n");
	    pthread_exit((void *)-1);
        }

        if ((int)mwuargs[1])
	    map_type = MAP_PRIVATE;
	else
	    map_type = MAP_SHARED;
        if ((map_address = mmap(0, (size_t)fsize,  PROT_WRITE|PROT_READ,
				map_type, (int)fd, 0))
			 == (caddr_t *) -1)
        {
            perror("map_write_unmap(): mmap()");
            pthread_exit((void *)-1);
        }

        memset(map_address, 'A', fsize);

        fprintf(stdout,
		"Map address = %p\nNum iter: [%d]\nTotal Num Iter: [%d]",
		map_address, mwu_ndx, (int)mwuargs[0]);
	usleep(1);
        if (munmap(map_address, (size_t)fsize) == -1)
        {
	    perror("map_write_unmap(): mmap()");
            pthread_exit((void *)-1);
        }
        close (fd);
    }
    pthread_exit((void *)0);
}

/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Descrption:	Create a large file of size up to a  Giga Bytes.  write to it */
/*		lower case alphabet 'a'. Map the file and change the contents */
/*		to 'A's (upper case alphabet), write the contents to the file,*/
/*		and unmap the file from memory. Spwan a certian number of     */
/*		LWP's that will do the above.				      */
/*                                                                            */
/* Return:	exits with -1 on error					      */
/*		exits with a 0 on success.				      */
/*                                                                            */
/******************************************************************************/
int
main(int  argc,		/* number of input parameters.			      */
     char **argv)	/* pointer to the command line arguments.	      */
{
    int 	 c;		/* command line options			      */
    int		 num_iter;	/* number of iteration to perform             */
    int		 num_thrd;	/* number of threads to create                */
    int		 thrd_ndx;	/* index into the array of threads.	      */
    float	 exec_time;	/* period for which the test is executed      */
    int          status;       /* exit status for light weight process       */
    int          sig_ndx;      	/* index into signal handler structure.       */
    pthread_t    thid[1000];	/* pids of process that will map/write/unmap  */
    long         chld_args[3];	/* arguments to funcs execed by child process */
    extern  char *optarg;	/* arguments passed to each option	      */
    struct sigaction sigptr;	/* set up signal, for interval timer          */
    int          map_private =  /* if TRUE mapping is private, ie, MAP_PRIVATE*/
			       FALSE;

    static struct signal_info
    {
        int  signum;    /* signal number that hasto be handled                */        char *signame;  /* name of the signal to be handled.                  */    } sig_info[] =
                   {
			   {SIGHUP,"SIGHUP"},
			   {SIGINT,"SIGINT"},
			   {SIGQUIT,"SIGQUIT"},
			   {SIGABRT,"SIGABRT"},
			   {SIGBUS,"SIGBUS"},
			   {SIGSEGV,"SIGSEGV"},
			   {SIGALRM, "SIGALRM"},
			   {SIGUSR1,"SIGUSR1"},
			   {SIGUSR2,"SIGUSR2"},
			   {-1,     "ENDSIG"}
                   };

    /* set up the default values */
    num_iter = 1000;	/* repeate map - write - unmap operation 1000 times   */
    num_thrd = 40;	/* number of LWP's to create			      */
    exec_time = 24;	/* minimum time period for which to run the tests     */

    while ((c =  getopt(argc, argv, "h:l:n:px:")) != -1)
    {
        switch(c)
	{
	    case 'h':
		usage(argv[0]);
		break;
	    case 'l':
		if ((num_iter = atoi(optarg)) == 0)
		    num_iter = 1000;
		break;
	    case 'n':
		if ((num_thrd = atoi(optarg)) == 0)
		    num_thrd = 20;
		break;
	    case 'p':
                map_private = TRUE;
		break;
	    case 'x':
		if ((exec_time = atof(optarg)) == 0)
		    exec_time = 24;
		break;
	    default :
		usage(argv[0]);
		break;
        }
    }

    /* set up signals */
    sigptr.sa_handler = (void (*)(int signal))sig_handler;
    sigfillset(&sigptr.sa_mask);
    sigptr.sa_flags = SA_SIGINFO;
    for (sig_ndx = 0; sig_info[sig_ndx].signum != -1; sig_ndx++)
    {
        sigaddset(&sigptr.sa_mask, sig_info[sig_ndx].signum);
        if (sigaction(sig_info[sig_ndx].signum, &sigptr,
                (struct sigaction *)NULL) == -1 )
        {
            perror( "man(): sigaction()" );
            fprintf(stderr, "could not set handler for SIGALRM, errno = %d\n",
                    errno);
            exit(-1);
        }
    }
    chld_args[0] = num_iter;
    chld_args[1] = map_private;
    set_timer(exec_time);

    fprintf(stdout, "\n\n\nTest is set to run with the following parameters:\n"
		    "\tDuration of test: [%f]hrs\n"
		    "\tNumber of threads created: [%d]\n"
		    "\tnumber of map-write-unmaps: [%d]\n"
 		    "\tmap_private?(T=1 F=0): [%d]\n\n\n\n", exec_time,
			num_thrd, num_iter, map_private);

    for (;;)
    {
        /* create num_thrd number of threads. */
        for (thrd_ndx = 0; thrd_ndx<num_thrd; thrd_ndx++)
        {
            if (pthread_create(&thid[thrd_ndx], NULL, map_write_unmap,
			chld_args))
            {
                perror("main(): pthread_create()");
                exit(-1);
            }
            sched_yield();
        }

        /* wait for the children to terminate */
        for (thrd_ndx = 0; thrd_ndx<num_thrd; thrd_ndx++)
        {
            if (pthread_join(thid[thrd_ndx], (void *)&status))
            {
                perror("main(): pthread_create()");
                exit(-1);
            }
            else
            {
                if (status)
                {
                    fprintf(stderr,
			    "thread [%d] - process exited with errors %d\n",
			        WEXITSTATUS(status), status);
	            exit(-1);
                }
            }
        }
    }
    exit(0);
}