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
/*                                                                            */
/* History:     Nov - 04 - 2001 Created - Manoj Iyer, IBM Austin TX.          */
/*                               email:manjo@austin.ibm.com                   */
/*                                                                            */
/*		Nov - 06 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- added function alloc_mem()                  */
/*                                                                            */
/******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXL    1000    /* default number of loops to do malloc and free      */
#define MAXT    30      /* default number of threads to create.               */

#ifdef DEBUG
#define dprt	printf
#else
#define dprt
#endif

#define PTHREAD_EXIT(val)    do {\
			exit_val = val; \
                        dprt("pid[%d]: exiting with %d\n", getpid(),exit_val); \
			pthread_exit((void *)&exit_val); \
				} while (0)

#define OPT_MISSING(prog, opt)   do{\
			       fprintf(stderr, "%s: option -%c ", prog, opt); \
                               fprintf(stderr, "requires an argument\n"); \
                               usage(prog); \
                                   } while (0)

/******************************************************************************/
/*								 	      */
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
               "Usage: %s -d NUMDIR -f NUMFILES -h -t NUMTHRD\n"
               "\t -h Help!\n"
               "\t -l Number of loops:               Default: 1000\n"
               "\t -t Number of threads to generate: Default: 30\n",
                    progname);
    exit(-1);
}



/******************************************************************************/
/* Function:	alloc_mem				                      */
/*								              */
/* Description:	This thread function will allocate and free memory by calling */
/*		malloc and free functions. The function will generate a random*/
/* 		number and depending on the number generated, it will chose   */
/* 		between the three algorythims to calculate how much it should */
/*		allocate. It will allocate fibannoci series, power of 2 or    */
/* 		power of 3 size memory.					      */
/******************************************************************************/
void *
alloc_mem(void * args)
{
    int	 fib1       = 0;	/* first number in the fibannoci series       */
    int  fib2       = 1;	/* second number in the fibanocci series      */
    int  fib3       = 1;	/* third number in the fibanocci series       */
    int  num_alloc  = 0;	/* number of memory chunks allocated          */
    int  index      = 0;        /* num of and frees to perform                */
    int  loop       = 0;        /* number of times to repeat alloc and free   */
    long **memptr;		/* array of pointers to the memory allocated  */
    long **anchor;		/* save this pointer in anchor                */
    long *locargptr =           /* local pointer to thread arguments          */
                      (long *)args;
    volatile int exit_val = 0;  /* exit value of the pthreads                 */

    for (loop = 0; loop < (int)locargptr[0]; loop++)
    {
        dprt("mallocing memory for anchor\n");
        if ((anchor = malloc(sizeof(int *))) == NULL)
        {
            perror("do_malloc(): allocating space for anchor malloc()");
            PTHREAD_EXIT(-1);
        }

        memptr = anchor;
        dprt("malloc done. anchor = %#x memptr = %#x\n", anchor, memptr);

        while ((*memptr = (int *)malloc(fib3)) != NULL)
        {
            dprt("fib1 = %d fib2 = %d and fib3 = %d\n", fib1, fib2, fib3);

            fib3 = fib2 + fib1;
            fib1 = fib2;
            fib2 = fib3;
       
            num_alloc++;
            **memptr = num_alloc;
            memptr = anchor + num_alloc;
            
            dprt("content of memptr = %d\n", **memptr);
            dprt("number of allocations = %d\n", num_alloc);

            if ((anchor = 
                        (int **) realloc(anchor, (num_alloc + 2)*sizeof(int *)))
		        == NULL)
            {
                perror("do_malloc(): reallocating space for anchor malloc()");
                PTHREAD_EXIT(-1);
            }
            dprt("anchor remalloced... anchor = %#x\n", anchor);
        }
        for (index = 0; index < num_alloc; index++)
        {
            free(*memptr);
            memptr--;
        }

        free(anchor);
    }   
}

/******************************************************************************/
/*								 	      */
/* Function:	main							      */
/*									      */
/* Description:	This is the entry point to the program. This function will    */
/*		parse the input arguments and set the values accordingly. If  */
/*		no arguments (or desired) are provided default values are used*/
/*		refer the usage function for the arguments that this program  */
/*		takes. It also creates the threads which do most of the dirty */
/*		work. If the threads exits with a value '0' the program exits */
/*		with success '0' else it exits with failure '-1'.             */
/*									      */
/* Return:	-1 on failure						      */
/*		 0 on success						      */
/*									      */
/******************************************************************************/
main(int	argc,		/* number of input parameters		      */
     char	**argv)		/* pointer to the command line arguments.     */
{
    int		c;		/* command line options			      */
    int		num_thrd = MAXT;/* number of threads to create                */
    int		num_loop = MAXL;/* number of loops to perform		      */
    int		thrd_ndx;	/* index into the array of thread ids         */
    int		*th_status;	/* exit status of LWP's	                      */
    pthread_t	thrdid[30];	/* maxinum of 30 threads allowed              */
    long	chld_args[3];   /* arguments to the thread function           */
    extern char* optargs;	/* pointer to arguments for each option flag  */
    extern int	 optopt;	/* options to the program		      */

    while ((c =  getopt(argc, argv, "hl:t:")) != -1)
    {
        switch(c)
        {
            case 'h':
                usage(argv[0]);
                break;
            case 'l':
		if ((num_loop = atoi(optarg)) == (int)NULL)
	            OPT_MISSING(argv[0], optopt);
                else
                if (num_loop < 0)
                {
                    fprintf(stdout,
                        "WARNING: bad argument. Using default\n");
                    num_loop = MAXL;
                }
                break;
            case 't':
		if ((num_thrd = atoi(optarg)) == (int)NULL)
	            OPT_MISSING(argv[0], optopt);
                else
                if (num_thrd < 0)
                {
                    fprintf(stdout,
                        "WARNING: bad argument. Using default\n");
                    num_thrd = MAXT;
                }
                break;
            default :
		usage(argv[0]);
		break;
	}
    }
    
    dprt("number of times to loop in the thread = %d\n", num_loop);
    chld_args[0] = num_loop;

    for (thrd_ndx = 0; thrd_ndx < num_thrd; thrd_ndx++)
    {
        if (pthread_create(&thrdid[thrd_ndx], NULL, alloc_mem, chld_args))
        {
            perror("main(): pthread_create()");
            exit(-1);
        }
    }
    
    sync();
    th_status = malloc(sizeof(int *));

    for (thrd_ndx = 0; thrd_ndx < num_thrd; thrd_ndx++)
    {
        if (pthread_join(thrdid[thrd_ndx], (void **)&th_status) != 0)
        {
            perror("main(): pthread_join()");
            exit(-1);
        }
        else
        {
            dprt("WE ARE HERE %d\n", __LINE__);
            if ((int)*th_status == -1)
            {
                fprintf(stderr,
                        "thread [%d] - process exited with errors\n",
                            thrdid[thrd_ndx]);
                exit(-1);
            }
        }
    }
    exit(0);
}
