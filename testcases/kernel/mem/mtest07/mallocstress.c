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
/*    								              */
/*		Nov - 08 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- added logic to allocate memory in the size  */
/*				  of fibanocci numbers.                       */
/*				- fixed segmetation fault.                    */
/*									      */
/*		Nov - 09 - 2001 Modified - Manoj Iyer, IBM Austin TX.         */
/*				- separated alocation logic to allocate_free()*/
/*				  function.                                   */
/*				- introduced logic to randomly pick allocation*/
/*				  scheme. size = fibannoci number, pow of 2 or*/
/*				  power of 3.                                 */
/*				- changed comments.                           */
/*				- Added test to LTP.                          */
/*                                                                            */
/* File:	mallocstress.c						      */
/*									      */
/* Description:	This program is designed to stress the VMM by doing repeated  */
/*		mallocs and frees, with out using the swap space. This is     */
/*		achived by spawnning N threads with repeatedly malloc and free*/
/*		a memory of size M. The stress can be increased by increasing */
/*		the number of repetations over the default number using the   */
/*		-l [num] option.					      */
/******************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define MAXL    100     /* default number of loops to do malloc and free      */
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
#define FOREVER 1

/******************************************************************************/
/*								 	      */
/* Function:	usage							      */
/*									      */
/* Description:	Print the usage message.				      */
/*									      */
/* Input:	char *progname - name of this program                         */
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
/* Function:	allocate_free				                      */
/*								              */
/* Description:	This function does the allocation and free by calling malloc  */
/*		and free fuctions. The size of the memory to be malloced is   */
/*		determined by the caller of this function. The size can be    */
/*		a number from the fibannoaci series, power of 2 or 3 or 7     */
/*									      */
/* Input:	int repeat - number of times the alloc/free is repeated.      */
/*		int topower - power of the number that has to be calculated.  */
/*								              */
/* Return:	1 on failure						      */
/*		0 on success						      */
/******************************************************************************/
int
allocate_free(int    repeat,	/* number of times to repeat allocate/free    */
              int    topow)	/* caculate the power                         */
{
    int  loop = 0;		/* index to the number of allocs and frees    */
    long **memptr;		/* array of pointers to the memory allocated  */
    long **anchor;		/* save this pointer in anchor                */

    for (loop = 0; loop < repeat; loop++)
    {
        int     fib1       = 0;	    /* first number in the fibannoci series   */
        int     fib2       = 1;	    /* second number in the fibanocci series  */
        int     fib3       = 1;	    /* third number in the fibanocci series   */
        int     num_alloc  = 0;	    /* number of memory chunks allocated      */
        int     index      = 0;     /* num of and frees to perform            */
        int     size       = topow; /* size of mem to malloc or free          */
 
        dprt("pid[%d]: In for loop = %d times\n", getpid(), loop);

        if ((anchor = malloc((size_t)sizeof(int *))) == NULL)
        {
            perror("do_malloc(): allocating space for anchor malloc()");
            return 1;
        }

        dprt("pid[%d]: loop = %d anchor = %#x\n", getpid(), loop, anchor);

        while(FOREVER)
        {
            dprt("pid[%d]: In for loop = %d\n", getpid(), loop); 
            dprt("pid[%d]: In while loop = %d\n", getpid(), num_alloc);
            memptr = anchor + num_alloc;
            if ((*memptr = (int *)malloc((size_t)size)) == NULL)
            {
                perror("alloc_mem(): malloc()");
                fprintf(stdout, "pid[%d]: malloc failed on size %d\n", getpid(),
			    size);
                break;
            }

            dprt("pid[%d]: memptr = %#x\n", getpid(), *memptr);
            
            if (!topow)
            {
                fib3 = fib2 + fib1;
                fib1 = fib2;
                fib2 = fib3;
                size = fib3;
            }
            else 
            {
                size = (topow == 2) ? (size * 2) : (size * 3);
            }

            num_alloc++;
            **memptr = num_alloc;
            
            dprt("pid[%d]: malloc size = %d\n", getpid(), size);
            dprt("pid[%d]: content of memptr = %d\n", getpid(), **memptr);
            dprt("pid[%d]: number of allocations = %d\n", getpid(), num_alloc);

            if ((anchor = 
                        (int **) realloc(anchor, (num_alloc + 2)*sizeof(int *)))
		        == NULL)
            {
                perror("do_malloc(): reallocating space for anchor malloc()");
                return 1;
            }
            usleep(0);
            dprt("pid[%d]: remalloced anchor = %#x\n", getpid(), anchor);
        }

        for (index = 0; index < num_alloc; index++)
        {
            dprt("pid[%d]: freeing *memptr %#x\n", getpid(), *memptr);
            free(*memptr);
            memptr--;
            usleep(0);
        }

        dprt("pid[%d]: freeing anchor %#x\n", getpid(), anchor);
        free(anchor);
        usleep(0);
    }   
    fprintf(stdout, "pid[%d]: exiting sucessfully\n", getpid());
    return 0;
}


/******************************************************************************/
/* Function:	alloc_mem				                      */
/*								              */
/* Description:	This thread function will allocate and free memory.           */
/*		This function will pick a random scheme to calculate memory   */
/*		size that needs to be allocated. size is calculated as a      */
/*		number from the fibannoci series, 3*3*3..., 2*2*2... or 7*7*..*/
/*									      */
/* Input:	args - arg[0] is the numof times the allocation and free is to*/
/*			      be repeated.			              */
/*								              */
/* Return:	pthread_exit -1	on failure				      */
/*		pthread_exit  0 on success			              */
/*								              */
/******************************************************************************/
void *
alloc_mem(void * args)
{
    int random	          = 0;  /* random number 			      */
    volatile int exit_val = 0;  /* exit value of the pthreads                 */
    long *locargptr =           /* local pointer to thread arguments          */
                      (long *)args;

    srand(time(NULL)%100);
    random = (1 + (int)(10.0*rand()/(RAND_MAX+1.0)));
    
    if (!(random % 2))
    {
        fprintf(stdout, 
                 "pid[%d]: allocating memory of size = fibnonnaci number\n");
        if (allocate_free((int)locargptr[0], 0))
        {
            fprintf(stdout, "pid[%d]: alloc_mem(): fib_alloc() failed\n");
            PTHREAD_EXIT(-1);
        }
        else
        {
            fprintf(stdout, "pid[%d]: Thread exiting. Task complete.\n");
            PTHREAD_EXIT(0);
        }
    }
            
    usleep(0);
    if (!(random % 3))
    {
        fprintf(stdout, 
                 "pid[%d]: allocating memory of size = the power of 2\n");
        if (allocate_free((int)locargptr[0], 2))
        {
            fprintf(stdout, "pid[%d]: alloc_mem(): allocate_free() failed\n");
            PTHREAD_EXIT(-1);
        }
        else
        {
            fprintf(stdout, "pid[%d]: Thread exiting. Task complete.\n");
            PTHREAD_EXIT(0);
        }
    }
    usleep(0);
    if (!(random % 5))
    {
        fprintf(stdout, 
                 "pid[%d]: allocating memory of size = the power of 3\n");
        if (allocate_free((int)locargptr[0], 3))
        {
            fprintf(stdout, "pid[%d]: alloc_mem(): allocate_free() failed\n");
            PTHREAD_EXIT(-1);
        }
        else
        {
            fprintf(stdout, "pid[%d]: Thread exiting. Task complete.\n");
            PTHREAD_EXIT(0);
        }
    }
    usleep(0);

    /* default  power of "seven"*/
    fprintf(stdout, 
             "pid[%d]: allocating memory of size = the power of 7\n");
    if (allocate_free((int)locargptr[0], 7))
    {
        fprintf(stdout, "pid[%d]: alloc_mem(): allocate_free() failed\n");
        PTHREAD_EXIT(-1);
    }
    else
    {
        fprintf(stdout, "pid[%d]: Thread exiting. Task complete.\n");
        PTHREAD_EXIT(0);
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
    
    usleep(0);
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
            if ((int)*th_status == -1)
            {
                fprintf(stderr,
                        "main(): thread [%d] - process exited with errors\n",
                            thrdid[thrd_ndx]);
                exit(-1);
            }
            dprt("main(): thread[%d]: exiting without errors\n", thrd_ndx);
        }
        usleep(0);
    }
    exit(0);
}
