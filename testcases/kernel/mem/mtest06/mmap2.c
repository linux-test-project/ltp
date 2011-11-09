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
/* History:	July - 02 - 2001 Created by Manoj Iyer, IBM Austin TX.	      */
/*			         email:manjo@austin.ibm.com		      */
/*									      */
/*		July - 07 - 2001 Modified - changed MAP_PRIVATE to MAP_SHARED */
/*			         read defect 187 for details.	              */
/*									      */
/*		July - 09 - 2001 Modified - added option to MAP_PRIVATE or    */
/*				 MAP_SHARED, -p, default is to MAP_SHARED.    */
/*									      */
/*		July - 09 - 2001 Modified - added option '-a' MAP_ANONYMOUS.  */
/*                               Default is to map a file.		      */
/*									      */
/*		Aug  - 01 - 2001 Modified - added option 'a' to getop list.   */
/*									      */
/*		Oct  - 25 - 2001 Modified - changed scheme. Test will be run  */
/*				 once unless -x option is used.               */
/*									      */
/*		Apr  - 16 - 2003 Modified - replaced tempnam() use with       */
/*				 mkstemp(). -Robbie Williamson                */
/*				 email:robbiew@us.ibm.com                     */
/*									      */
/*		May  - 12 - 2003 Modified - remove the huge files when        */
/*				 we are done with the test - Paul Larson      */
/*				 email:plars@linuxtestproject.org             */
/* File:	mmap2.c							      */
/*			         					      */
/* Description: Test the LINUX memory manager. The program is aimed at        */
/*              stressing the memory manager by repeaded map/write/unmap of a */
/*		of a large gb size file.				      */
/*			         					      */
/*		Create a file of the specified size in gb, map the file,      */
/*		change the contents of the file and unmap it. This is repeated*/
/*		several times for the specified number of hours.	      */
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
#include <signal.h>
#include <string.h>
#include "test.h"
#include "usctest.h"

/* Defines								      */
#define GB 1000000000
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/******************************************************************************/
/*                                                                            */
/* Function:    mkfile                                                        */
/*                                                                            */
/* Description: Create a temparory file of required size. 		      */
/*                                                                            */
/* Input:	size - size of the temp file to be created in giga bytes      */
/*                                                                            */
/* Return:      int fd - file descriptor if the file was created.             */
/*              -1     - if it failed to create.                              */
/*                                                                            */
/******************************************************************************/
static int
mkfile(int size) 		/* size of the file to be generated in GB     */
{
    int  fd;			/* file descriptior of tmpfile		      */
    int  index = 0;		/* index into the file, fill it with a's      */
    char buff[4096];		/* buffer that will be written to the file.   */
    char template[PATH_MAX];    /* template for temp file name                */

    /* fill the buffer with a's and open a temp file */
    memset(buff, 'a', 4096);
    snprintf(template, PATH_MAX, "ashfileXXXXXX");
    if ((fd = mkstemp(template)) == -1)
    {
	perror("mkfile(): mkstemp()");
	return -1;
    }
    else
    {
	unlink(template);
        fprintf(stdout, "creating tmp file and writing 'a' to it ");
    }

    /* fill the file with the character a */
    while (index < (size * GB))
    {
        index += 4096;
        if (write(fd, buff, 4096) == -1)
	{
	    perror("mkfile(): write()");
	    return -1;
	}
    }
    fprintf(stdout, "created file of size %d\n"
		    "content of the file is 'a'\n", index);

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
               "Usage: %s -h -s -x\n"
	       "\t -a set map_flags to MAP_ANONYMOUS\n"
               "\t -h help, usage message.\n"
               "\t -p set map_flag to MAP_PRIVATE.\tdefault: MAP_SHARED\n"
               "\t -s size of the file/memory to be mmaped.\tdefault: 1GB\n"
               "\t -x time for which test is to be run.\tdefault: 24 Hrs\n",
                    progname);
    exit(-1);
}

/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Descrption:	Create a large file of size in Giga Bytes. Fill it with 'a's  */
/*		(lower case alphabet a). Map the file and change the contents */
/*		to 'A's (upper case alphabet), write the contents to the file,*/
/*		and unmap the file from memory. Continue map/write/unmap      */
/*		operation for user specified number of times.		      */
/*                                                                            */
/* Return:	exits with -1 on error					      */
/*		exits with a 0 o success.				      */
/*                                                                            */
/******************************************************************************/
int main(int argc,	    /* number of input parameters.		      */
     char **argv)	    /* pointer to the command line arguments.         */
{
    int   fd;		    /* descriptor of temp file.		              */
    int   fsize = 1;	    /* size of the temp file created. default 1GB     */
    float exec_time = 24;    /* period of execution, default 24 hours.	      */
    int   c;		    /* command line options			      */
    int   sig_ndx;	    /* index into signal handler structure.	      */
    int   map_flags =        /* type of mapping, defaut is MAP_SHARED .	      */
		     MAP_SHARED;
    int   map_anon =  FALSE; /* do not map anonymous memory,map file by default*/
    int   run_once = TRUE;   /* run the test once. (dont repeat)               */
    char  *memptr;	    /* address of the mapped file.	              */
    extern  char *optarg;   /* arguments passed to each option                */
    struct sigaction sigptr;/* set up signal, for interval timer              */

    static struct signal_info
    {
        int  signum;	/* signal number that hasto be handled 		      */
	char *signame;  /* name of the signal to be handled.		      */
    } sig_info[] =
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

    while ((c =  getopt(argc, argv, "ahps:x:")) != -1)
    {
        switch(c)
        {
	    case 'a':
		map_anon = TRUE;
	        break;
            case 'h':
		usage(argv[0]);
		exit(-1);
		break;
	    case 'p':
		map_flags = MAP_PRIVATE;
		break;
	    case 's':
		if ((fsize = atoi(optarg)) == 0)
		    fprintf(stderr, "Using default fsize %d GB\n", fsize = 1);
		break;
            case 'x':
		if ((exec_time = atof(optarg)) == 0)
		    fprintf(stderr, "Using default exec time %f hrs",
		          exec_time = (float)24);
                run_once = FALSE;
		break;
	    default :
		usage(argv[0]);
		break;
	}
    }

    fprintf(stdout, "MM Stress test, map/write/unmap large file\n"
		    "\tTest scheduled to run for:       %f\n"
		    "\tSize of temp file in GB:         %d\n",
			exec_time, fsize);

    /* set up time for which test has to be run */
    alarm(exec_time * 3600.00);

    /* set up signals */
    sigptr.sa_handler = (void (*)(int signal))sig_handler;
    sigfillset(&sigptr.sa_mask);
    sigptr.sa_flags = 0;
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

    do
    {
        if (!map_anon)
        {
            /* create a new file of giga byte size */
            if ((fd = mkfile(fsize)) == -1)
            {
 	        fprintf(stderr,
			     "main(): mkfile(): Failed to create temp file.\n");
 	        exit (-1);
            }
        }
        else
        {
            /* mapping anonymous,  MAP_SHARED or MAP_PRIVATE must be also set */
            fd = -1;
	    map_flags = map_flags|MAP_ANONYMOUS;
        }

        if ((memptr = (char *)mmap(0,(fsize * GB), PROT_READ|PROT_WRITE,
		     map_flags, fd, 0)) == (char *)-1)
        {
            perror("main(): mmap()");
            exit (-1);
        }
        else
	    fprintf(stdout, "file mapped at %p\n"
			    "changing file content to 'A'\n", memptr);

        /* Change the content of the file with A's, and commit changes */
        memset(memptr, 'A', ((fsize * GB)/sizeof(char)));

        if (msync(memptr, ((fsize * GB)/sizeof(char)),
	                 MS_SYNC|MS_INVALIDATE) == -1)
        {
            perror("main(): msync()");
            exit (-1);
        }

        if (munmap(memptr, (fsize * GB)/sizeof(char)) == -1)
        {
            perror("main(): munmap()");
            exit (-1);
        }
	else
	    fprintf(stdout, "unmapped file at %p\n", memptr);

	close(fd);
        sync();
    }while (TRUE && !run_once);
    exit (0);
}
