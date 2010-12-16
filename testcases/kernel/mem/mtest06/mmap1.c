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
/* 								              */
/* History:	June - 26 - 2001 Created  - Manoj Iyer, IBM Austin TX.        */
/*		                 email:manjo@austin.ibm.com		      */
/*								              */
/* 		June - 28 - 2001 Modified - use pthreads instead of clone bcoz*/
/*				 it seems to be a little shakey, and program  */
/*				 becomes unportable.			      */
/*								              */
/*		July - 09 - 2001 Modified - If a read happens after an unmap  */
/*				 ignore this conition and let the thread go on*/
/*				 sigsetjmp and siglongjmp calls added to      */
/*				 acomplish this.		              */
/*								              */
/*              July - 10 - 2001 Modified - corrected thread functions to use */
/*				 pthread_exit() instead of return!            */
/*								              */
/*		July - 24 - 2001 Modified - fixed function main so that the   */
/*				 test will run for a user specified duration  */
/*								              */
/*		Aug  - 01 - 2001 Modified - Added header files pthread.h      */
/*				 contains definitions for pthread_t etc.      */
/*						                              */
/*              Oct  - 24 - 2001 Modified - Fixed broken code in main()       */
/*			         pthread_join() part of the code was fixed to */
/*			         check for the correct return status from the */
/*			         thread function.                             */
/*						                              */
/*		Oct  - 25 - 2001 Modified - added OPT_MISSING                 */
/*		                 chaned scheme, test will run once unless the */
/*				 -x option is used.			      */
/*		Nov  - 09 - 2001 Modified - Removed compile errors            */
/*				 - too many to list!			      */
/*				 - major change - added header file string.h  */
/*				 - missing argument status[thrd_ndx] added    */
/*				   to printf in pthread_join() logic          */
/*                                                                            */
/*              Apr  - 16 - 2003 Modified - replaced tempnam() use with       */
/*                               mkstemp(). -Robbie Williamson                */
/*                               email:robbiew@us.ibm.com                     */
/*                                                                            */
/*              May  - 12 - 2003 Modified - removed the huge files when       */
/*                               we are done with the test - Paul Larson      */
/*                               email:plars@linuxtestproject.org             */
/*								              */
/*              Jun  - 12 - 2007 Modified - removing 'run_once', so the test  */
/*                               can be run only with a time value (or the    */
/*                               default one, 24 hours). Bug 1732287, pointed */
/*                               by jerrywei.                                 */
/*                               email:rsalveti@linux.vnet.ibm.com            */
/*								              */
/*		Jul  - 13 - 2007 Modified - use siginfo_t to detect the       */
/*				 faulting address. Now this works fine on all */
/*				 architectures. - Suzuki K P                  */
/*				 email:suzuki@in.ibm.com		      */
/*				 				              */
/*		Jul  - 18 - 2007 Modified - Add SA_NODEFER flag to avoid      */
/*                               the chances of missing a SIGSEGV handling.   */
/*				 Do not mask SIGSEGV - Suzuki K P             */
/*				 email:suzuki@in.ibm.com	              */
/*				 				              */
/* File: 	mmap1.c							      */
/*									      */
/* Description:	Test the LINUX memory manager. The program is aimed at        */
/*		stressing the memory manager by simultanious map/unmap/read   */
/*		by light weight processes, the test is scheduled to run for   */
/*		a mininum of 24 hours.					      */
/*									      */
/*		Create two light weight processes X and Y.                    */
/*		X - maps, writes  and unmap a file in a loop.	              */
/*		Y - read from this mapped region in a loop.		      */
/*	        read must be a success between map and unmap of the region.   */
/*									      */
/******************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sched.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ucontext.h>
#include <setjmp.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

#define M_SUCCESS 	0	/* exit code - on success, clone functions    */
#define MWU_FAIL 	1	/* error code - map_write_unmap()  on error   */
#define RM_FAIL 	1	/* error code - read_mem()  on error          */
#ifdef DEBUG
#define prtln()		printf("%d\n", __LINE__);
#else
#define prtln()
#endif
#ifndef TRUE
#define TRUE            1
#endif
#ifndef FALSE
#define FALSE           0
#endif
#define OPT_MISSING(prog, opt) do {\
                        fprintf(stderr, "%s: option -%c ", prog, opt); \
                        fprintf(stderr, "requires an argument\n");\
						usage(prog); \
                            } while (0)

int 	   verbose_print = FALSE;/* when called with -v print more info       */
caddr_t    *map_address;	/* address of the file mapped.	              */
sigjmp_buf jmpbuf;		/* argument to sigsetjmp and siglongjmp       */

char *TCID = "mmap1";
int TST_TOTAL = 1;

/******************************************************************************/
/*									      */
/* Function:	sig_handler						      */
/*									      */
/* Description:	handle SIGALRM raised by set_timer(), SIGALRM is raised when  */
/*		the timer expires. If any other signal is recived exit the    */
/*		test.							      */
/*									      */
/* Input:	signal - signal number, intrested in SIGALRM!		      */
/*									      */
/* Return:	exit -1 if unexpected signal is recived			      */
/*		exit 0 if SIGALRM is recieved			              */
/*									      */
/******************************************************************************/
static void
sig_handler(int signal,		/* signal number, set to handle SIGALRM       */
            siginfo_t *info,
            struct ucontext *ut)/* contains pointer to sigcontext structure   */
{
    switch (signal)
    {
        case SIGALRM:
             tst_resm(TPASS, "Test ended, success");
             _exit(0);

        case SIGSEGV:
             if (info->si_code == SEGV_MAPERR &&
                      info->si_addr == map_address)
             {
                  tst_resm(TINFO,
                    "page fault occurred at %p",
                    map_address);
                  longjmp(jmpbuf, 1);
             }
        default:
            fprintf(stderr, "caught unexpected signal - %d --- exiting\n",
                     signal);
            _exit(-1);
    }
}

/******************************************************************************/
/*									      */
/* Function:	set_timer						      */
/*									      */
/* Description:	set up a timer to user specified number of hours. SIGALRM is  */
/*		raised when the timer expires.			              */
/*									      */
/* Input:	run_time - number of hours the test is intended to run.	      */
/*									      */
/* Return:	NONE							      */
/*									      */
/******************************************************************************/
static void
set_timer(double run_time)		/* period for which test is intended to run   */
{
    struct itimerval timer;	/* timer structure, tv_sec is set to run_time */

    memset(&timer, 0, sizeof(struct itimerval));
    timer.it_interval.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_value.tv_sec = (time_t)(run_time * 3600);

    if (setitimer(ITIMER_REAL, &timer, NULL))
    {
        perror("set_timer(): setitimer()");
	exit(-1);
    }
}

/******************************************************************************/
/*									      */
/* Function:	mkfile							      */
/*									      */
/* Description:	Create a temparory file of required size. Default directory . */
/*									      */
/* Input:	size of the file to be created.		                      */
/*									      */
/* Output:	name of the file created.			              */
/*									      */
/* Return:	int fd - file descriptor if the file was created.	      */
/*		-1     - if it failed to create.			      */
/*									      */
/******************************************************************************/
int
mkfile(int size	/* size of the temp file that needs to be created.    */ )
{
    int  fd;			/* file descriptor of the temp file created.  */
    char template[PATH_MAX];    /* template for temp file name		      */

    snprintf(template, PATH_MAX, "ashfileXXXXXX");

    /* open the file for writing and write to the required length. */
   if ((fd = mkstemp(template)) == -1)
    {
	perror("mkfile(): mkstemp()");
	return -1;
    }
    else
    {
	unlink(template);
        if (lseek(fd,  (size - 1), SEEK_SET) == -1)
        {
            perror("mkfile(): lseek()");
	    fprintf(stderr, "fd = %d size = %d\n", fd, size);
	    return -1;
        }

        if (write(fd, "\0", 1) == -1)
	{
	    perror("mksize(): write()");
	    return -1;
        }

	if (fsync(fd) == -1)
        {
            perror("mkfile(): fsync()");
	    return -1;
        }

        return fd;
   }
}

/******************************************************************************/
/*									      */
/* Function:	map_write_unmap						      */
/*									      */
/* Description:	map a file write a character to it and unmap, this is done for*/
/*		user defined number of times.				      */
/*									      */
/* Input:	arg[0]	  	   file descriptor of the file to be mapped.  */
/*              arg[1]	           size of the file mmaped.		      */
/*	        arg[2]		   number of times map - write -unmap is done */
/*									      */
/* Return:	MWU_FAIL on error.				              */
/*              MWU_SUCCESS on error less completion of the loop.             */
/*									      */
/******************************************************************************/
void *
map_write_unmap(void *args)	/* file descriptor of the file to be mapped.  */
{
    int     mwu_ndx = 0;	/* index to number of map/write/unmap         */
    long    *mwuargs =          /* local pointer to arguments		      */
		       (long *)args;
    long exit_val = 0;  /* exit value of the pthread		      */

    tst_resm(TINFO, "pid[%d]: map, change contents, unmap files %d times",
		getpid(), (int)mwuargs[2]);
    if (verbose_print)
        tst_resm(TINFO, "map_write_unmap() arguments are: "
		    "fd - arg[0]: %d; "
		    "size of file - arg[1]: %ld; "
		    "num of map/write/unmap - arg[2]: %d",
		    (int)mwuargs[0], (long unsigned)mwuargs[1], (int)mwuargs[2]);

    while (mwu_ndx++ < (int)mwuargs[2])
    {
        if ((map_address = mmap(0, (size_t)mwuargs[1],  PROT_WRITE|PROT_READ,
				MAP_SHARED, (int)mwuargs[0], 0))
			 == (caddr_t *) -1)
        {
            perror("map_write_unmap(): mmap()");
            exit_val = MWU_FAIL;
            pthread_exit((void *)exit_val);
        }

        if (verbose_print)
            tst_resm(TINFO, "map address = %p", map_address);

	prtln();

        memset(map_address, 'a', mwuargs[1]);

        if (verbose_print)
            tst_resm(TINFO, "[%d] times done: of total [%d] iterations, "
			"map_write_unmap():memset() content of memory = %s",
			mwu_ndx, (int)mwuargs[2], (char*)map_address);
	usleep(1);
        if (munmap(map_address, (size_t)mwuargs[1]) == -1)
        {
	    perror("map_write_unmap(): mmap()");
            exit_val = MWU_FAIL;
            pthread_exit((void *)exit_val);
        }
    }
    exit_val = M_SUCCESS;
    pthread_exit((void *)exit_val);
}

/******************************************************************************/
/*									      */
/* Function:	read_mem						      */
/*									      */
/* Description:	read the memory region that was written to by the process X   */
/*		process X writes 1's to this mapped file, check if this is    */
/*		is what process Y is reading.   		              */
/*									      */
/* Input:	arg[2]		number of reads to be performed		      */
/*									      */
/* Return:	RMEM_FAIL on error.				              */
/*              M_SUCCESS on error less completion of the loop.               */
/*									      */
/******************************************************************************/
void *
read_mem(void *args)		/* number of reads performed		      */
{
    static int	 rd_index = 0;	/* index to the number of reads performed.    */
    long 	*rmargs = args;	/* local pointer to the arguments	      */
    long exit_val = 0;  /* pthread exit value			      */

    tst_resm(TINFO, "pid[%d] - read contents of memory %p %ld times",
           getpid(), map_address, rmargs[2]);
    if (verbose_print)
        tst_resm(TINFO,
		"read_mem() arguments are: "
		"number of reads to be performed - arg[2]: %d; "
                "read from address %p",
		(int)rmargs[2], (long *)map_address);

    while (rd_index++ < (int)rmargs[2])
    {
        if (verbose_print)
	    tst_resm(TINFO,
	        "read_mem() in while loop  %d times to go %ld times",
		rd_index, rmargs[2]);

        if (setjmp(jmpbuf) == 1)
        {
            if (verbose_print)
	        tst_resm(TINFO,
		    "page fault occurred due to a read after an unmap from %p",
		    map_address);
        }
        else
        {
	    if (verbose_print)
	        tst_resm(TINFO,
		    "read_mem(): content of memory: %s", (char *)map_address);
            if (strncmp((char *)map_address, "a", 1) != 0)
            {
                exit_val = -1;
		pthread_exit((void *)exit_val);
            }
            usleep(1);
	}

    }
    exit_val = M_SUCCESS;
    pthread_exit((void *)exit_val);
}

/******************************************************************************/
/*									      */
/* Function:	main     						      */
/*									      */
/* Description: creates two light weight processes X and Y		      */
/*		process X map a file writes to it and unmaps the file for a   */
/*		specified number of times. Process Y reads from this region   */
/*		and validates the data for same number of loops. This is      */
/*		repeated for a default 24  hours or user specified number of  */
/*		hours.							      */
/*									      */
/* Return:	exits with -1 on error			                      */
/*              exits with a 0 on success				      */
/*									      */
/******************************************************************************/
static void
usage(char *progname)		/* name of this program			      */
{
    fprintf(stderr, "Usage: %s -d -l -s -v -x\n"
		    "\t -h help, usage message.\n"
		    "\t -l number of mmap/write/unmap     default: 1000\n"
		    "\t -s size of the file to be mmapped default: 1024 bytes\n"
		    "\t -v print more info.               default: quiet\n"
		    "\t -x test execution time            default: 24 Hrs\n",
			    progname);
    exit(-1);
}

/******************************************************************************/
/*									      */
/* Function:	main     						      */
/*									      */
/* Description: creates two light weight processes X and Y		      */
/*		process X map a file writes to it and unmaps the file for a   */
/*		specified number of times. Process Y reads from this region   */
/*		and validates the data for same number of loops. This is      */
/*		repeated for a default 24  hours or user specified number of  */
/*		hours.							      */
/*									      */
/* Return:	exits with -1 on error			                      */
/*              exits with a 0 on success				      */
/*									      */
/******************************************************************************/
int
main(int  argc,		/* number of input parameters.			      */
     char **argv)	/* pointer to the command line arguments.	      */
{

    extern char *optarg;/* getopt() function global variables         */
    extern int   optopt;

    int 	 c;		/* command line options			      */
    int		 thrd_ndx = 0;  /* index into the number of thrreads.         */
    int		 file_size;	/* size of the file to be created.	      */
    int		 num_iter;	/* number of iteration to perform             */
    double       exec_time;	/* period for which the test is executed */
    int		 fd;		/* temporary file descriptor		      */
    int          status[2];     /* exit status for process X and Y	      */
    int          sig_ndx;      	/* index into signal handler structure.       */
    pthread_t    thid[2];	/* pids of process X and Y		      */
    long         chld_args[3];	/* arguments to funcs execed by child process */
    extern  char *optarg;	/* arguments passed to each option	      */
    struct sigaction sigptr;	/* set up signal, for interval timer          */

    static struct signal_info
    {
        int  signum;    /* signal number that has to be handled               */
	char *signame;  /* name of the signal to be handled.                  */
    } sig_info[] =
                   {
			{ SIGHUP,"SIGHUP" },
                        { SIGINT,"SIGINT" },
                        { SIGQUIT,"SIGQUIT" },
                        { SIGABRT,"SIGABRT" },
                        { SIGBUS,"SIGBUS" },
                        { SIGSEGV,"SIGSEGV" },
                        { SIGALRM, "SIGALRM" },
                        { SIGUSR1,"SIGUSR1" },
                        { SIGUSR2,"SIGUSR2" },
                        { -1,     "ENDSIG" }
                   };

    /* set up the default values */
    file_size = 1024;
    num_iter = 1000;
    exec_time = 24;

    while ((c =  getopt(argc, argv, "hvl:s:x:")) != -1)
    {
        switch(c)
	{
	    case 'h':
		usage(argv[0]);
		break;
	    case 'l':
		if ((num_iter = atoi(optarg)) == 0)
		OPT_MISSING(argv[0], optopt);
                else
	        if (num_iter < 0)
				fprintf(stdout,"WARNING: bad argument. Using default %d\n", (num_iter = 1000));
		break;
	    case 's':
		if ((file_size = atoi(optarg)) == 0)
		    OPT_MISSING(argv[0], optopt);
        else
	        if (file_size < 0)
				fprintf(stdout,"WARNING: bad argument. Using default %d\n", (file_size = 1024));
		break;
	    case 'v':
		verbose_print = TRUE;
                break;
	    case 'x':
                exec_time = atof(optarg);
		if (exec_time == 0)
	     	    OPT_MISSING(argv[0], optopt);
		else
	             if (exec_time < 0)
				fprintf(stdout,"WARNING: bad argument. Using default %.0f\n", (exec_time = 24));
		break;
	    default :
		usage(argv[0]);
		break;
        }
    }

    if (verbose_print)
        tst_resm(TINFO, "Input parameters are: "
                    "File size:  %d; "
                    "Scheduled to run:  %lf hours; "
                    "Number of mmap/write/read:  %d",
			file_size, exec_time, num_iter);
    set_timer(exec_time);

    /* set up signals */
    /* Do not mask SIGSEGV, as we are interested in handling it. */

    sigptr.sa_handler = (void (*)(int signal))sig_handler;
    sigfillset(&sigptr.sa_mask);
    sigdelset(&sigptr.sa_mask, SIGSEGV);
    sigptr.sa_flags = SA_SIGINFO | SA_NODEFER;
    for (sig_ndx = 0; sig_info[sig_ndx].signum != -1; sig_ndx++)
    {
        if (sigaction(sig_info[sig_ndx].signum, &sigptr,
                (struct sigaction *)NULL) == -1 )
        {
            perror( "man(): sigaction()" );
            fprintf(stderr, "could not set handler for %s, errno = %d\n",
                    sig_info[sig_ndx].signame, errno);
            exit(-1);
        }
    }

    do
    {
        /* create temporary file */
        if ((fd = mkfile(file_size)) == -1)
        {
	    fprintf(stderr,
			"main(): mkfile(): Failed to create temporary file\n");
            exit (-1);
        }
        else
        {
            if (verbose_print)
                tst_resm(TINFO, "Tmp file created");
        }

        /* setup arguments to the function executed by process X */
        chld_args[0] = fd;
        chld_args[1] = file_size;
        chld_args[2] = num_iter;

        /* create process X */
        if (pthread_create(&thid[0], NULL, map_write_unmap, chld_args))
        {
            perror("main(): pthread_create()");
            exit(-1);
        }
        else
        {
            tst_resm(TINFO, "created thread[%ld]", thid[0]);
        }
        sched_yield();
        if (pthread_create(&thid[1], NULL, read_mem, chld_args))
        {
            perror("main(): pthread_create()");
            exit(-1);
        }
        else
        {
            tst_resm(TINFO, "created thread[%ld]", thid[1]);
        }
        sched_yield();

        for (thrd_ndx = 0; thrd_ndx < 2; thrd_ndx++)
        {
            if (pthread_join(thid[thrd_ndx], (void *)&status[thrd_ndx]))
            {
                perror("main(): pthread_join()");
                exit(-1);
            }
            else
            {
                if (status[thrd_ndx])
                {
                    fprintf(stderr,
			    "thread [%ld] - process exited with errors %d\n",
			        thid[thrd_ndx], status[thrd_ndx]);
	            exit (-1);
	        }
            }
        }
        close(fd);
    }while (TRUE);
    exit (0);
}