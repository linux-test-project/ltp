/******************************************************************************/
/*                                                                            */
/* Copyright (c) International Business Machines  Corp., 2001                 */
/*                                                                            */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or          */
/* (at your option) any later version.                                        */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   */
/* for more details.                                                          */
/*                                                                            */
/* You should have received a copy of the GNU General Public License          */
/* along with this program;  if not, write to the Free Software		          */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*									                                          */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* File:    sched_getsetprio.c                                                */
/*                                                                            */
/* History: Aug 21 2002 Created - Manoj Iyer, IBM Austin TX.                  */
/*                                                                            */
/* Description: Simple program that will test some schedular system calls,    */
/*              the test will spawn a bunch of threads, the threads will try  */
/*              and change its priority to a random value between max and min */
/*              possible value, yield the processor to another thread and     */
/*              then check if the schedular honored that priority value.      */
/*                                                                            */
/******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>   
#include <sys/timeb.h>

#ifdef DEBUG
#define dprt    printf
#else
#define dprt
#endif
#define OPT_MISSING(prog, opt)   \
                 do{\
	                   fprintf(stderr, "%s: option -%c ", prog, opt); \
					   fprintf(stderr, "requires an argument\n"); \
					   usage(prog); \
			       } while (0)

#define PTHREAD_EXIT(val)        \
                 do {\
                        exit_val = val; \
                        dprt("pid[%d]: exiting with %d\n", getpid(),exit_val);\
                        pthread_exit((void *)exit_val); \
                    } while (0)
#define MAXT 60         /* Maximum number of threads to create                */

typedef struct {
    int exp_prio;
    int act_prio;
    int proc_num;
    int procs_id;
} task_str_t;

int verbose = 0;


/******************************************************************************/
/*                                                                            */
/* Function:    get_proc_num                                                  */
/*                                                                            */
/* Description: This function returns the processor number on which the       */
/*              process was last executed. The function reads this info from  */
/*              proc filesystem from the file /proc/<pid>/stat file.          */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Output:      Success - returns processor number the process last executed  */
/*              Failure - returns -1 and prints error message to screen       */
/*                                                                            */
/******************************************************************************/
static int
get_proc_num(void)
{
    int fd = -1;        /* file descriptor of the /proc/<pid>/stat file       */
    int fsize = -1;     /* size of the /proc/<pid>/stat file                  */
    char filename[256]; /* buffer to hold the string /proc/<pid>/stat         */
    char fbuff[512];    /* contains the contents of the stat file             */

    /* get the name of the stat file for this process */
    sprintf(filename, "/proc/%d/stat", getpid());

    /* open the stat file and read the contents to a buffer */
    if ((fd  = open(filename, O_RDONLY)) == (int)NULL)
    {
        perror("get_proc_num(): open()");
        return -1;
    }

    usleep(6);
    //usleep(5);
    sched_yield();

    if ((fsize = read(fd, fbuff, 512)) == -1)
    {
        perror("main(): read()");
        return -1;
    }

    close(fd);
    /* return the processor number last executed on. */
    return atoi(&fbuff[fsize - 2]);
}


/******************************************************************************/
/*                                                                            */
/* Function:    thread_func                                                   */
/*                                                                            */
/* Description: This function is executed by each thread, sets the priority   */
/*              of the calling thread to a random value between max priority  */
/*              and min priority, yields processor to another process and     */
/*              checks if its prioriy was changed.                            */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Output:      Success - exit with task_str_t pointer.                       */
/*              Failure - exits -1 and prints error message to screen         */
/*                                                                            */
/******************************************************************************/
void *
thread_func(void *args)        /* arguments to the thread function           */
{
    static int max_priority;    /* max possible priority for a process        */
    static int min_priority;    /* min possible priority for a process          */
    static int set_priority;    /* set the priority of the proc by this value */
    static int get_priority;    /* get the priority that is set for this proc */
    static int procnum;         /* processor number last executed on.         */
    volatile int exit_val = 0;  /* exit value of the pthreads, set to success */
    pid_t  ppid;                /* pid of the current process                 */
    struct sched_param ssp;     /* set schedule priority                      */
    struct sched_param gsp;     /* gsp schedule priority                      */
    struct timeb       tptr;    /* tptr.millitm will be used to seed srand    */
    task_str_t *locargptr =     /* local ptr to the arguments                 */
                    (task_str_t *) args;

    /* Get the system max and min static priority for a process. */
    if (((max_priority = sched_get_priority_max(SCHED_FIFO)) == -1) ||
         ((min_priority = sched_get_priority_min(SCHED_FIFO)) == -1))
    {
        fprintf(stderr, "failed to get static priority range\n");
        PTHREAD_EXIT(-1);
    }

    usleep(7);
    /* Set a random value between max_priority and min_priority */
    ftime(&tptr);
    srand((tptr.millitm)%1000);
    set_priority = (min_priority + (int)((float)max_priority 
            * rand()/(RAND_MAX+1.0)));

    ssp.sched_priority = set_priority;
    
    /* give other threads a chance */
    usleep(8);
    /* set a random priority value and check if this value was honoured. */
    if ((sched_setscheduler(getpid(), SCHED_FIFO, &ssp)) == -1)
    {
        perror("main(): sched_setscheduler()");
        PTHREAD_EXIT(-1);
    }

    /* give other threads a chance */
    sched_yield();
    if ((get_priority = sched_getparam(getpid(), &gsp)) == -1)
    {
        perror("main(): sched_setscheduler()");
        PTHREAD_EXIT(-1);
    }

    /* processor number this process last executed on */
    if ((procnum = get_proc_num()) == -1)
    {
        fprintf(stderr, "main(): get_proc_num() failed\n");
        PTHREAD_EXIT(-1);
    }

    if (verbose)
    {
        fprintf(stdout, 
        "Max priority             = %d\n"
        "Min priority             = %d\n"
            "Expected priority        = %d\n"
            "Actual assigned priority = %d\n"
            "Processor last execed on = %d\n\n",
         max_priority, min_priority, set_priority, 
         gsp.sched_priority, procnum);
    }

    locargptr->exp_prio = set_priority;
    locargptr->act_prio = gsp.sched_priority;
    locargptr->proc_num = procnum;
    locargptr->procs_id = getpid();

    PTHREAD_EXIT(locargptr);
}


/******************************************************************************/
/*                                                                            */
/* Function:    usage                                                         */
/*                                                                            */
/* Description: prints the usage message.                                     */
/*                                                                            */
/* Input:       program name                                                  */
/*                                                                            */
/* Output:      NONE                                                          */
/*                                                                            */
/******************************************************************************/
static void
usage(char *progname)         /* name of this program                         */
{
	fprintf(stderr,
			    "Usage: %s -h -n numthreads -p numprocessors -v \n"
				"\t -h HELP!\n"
				"\t -n Number of threads       Default: 1\n"
				"\t -p Number of processors    Default: 1\n"
				"\t -v verbose output\n", progname);
	exit(-1);
}


/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Description: entry point of the program, this function spawns a bunch of   */
/*              threads, waits for all the threads to exit before the program */
/*              exits.                                                        */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Output:      Success - exit with 0.                                        */
/*              Failure - exits -1 and prints error message to screen         */
/*                                                                            */
/******************************************************************************/
int
main(int  argc,        /* number of input parameters.                         */
     char **argv)      /* pointer to the command line arguments.              */
{
    int          c;             /* command line options                       */
    int          iter;          /* number of iterations to perform.           */
    int          num_thrd;      /* number of threads to create                */
    int          thrd_ndx;      /* index into the array of threads.           */
    int          *status;       /* exit status for light weight process       */
    int          proc_num;      /* processor number as returned by process    */
    int          pid_ndx;       /* index into the number of pids created.     */
    int          proc_id[512];  /* id of the processor last execed on.        */
    int          exp_prio[1000];/* priority expected to be set                */
    int          act_prio[1000];/* priority actually set by scheduler         */
    int          gen_pid[1000]; /* pid of the process/task generated          */
    int          chld_args[4];  /* arguments to funcs execed by child process */
    int          num_of_procs = 8;   /*** hack!!! make it autodetect later**/
    pthread_t    thid[1000];    /* pids of process that will map/write/unmap  */
    extern  char *optarg;       /* arguments passed to each option            */
	extern  int  optopt;        /* options to this program                    */
    task_str_t   *child_str;    /* collect stats, priority val, proc num etc  */

    while ((c =  getopt(argc, argv, "hl:t:")) != -1)
    {
        switch(c)
        {
            case 'h':
                usage(argv[0]);
                break;
            case 'p':
				if ((num_of_procs = atoi(optarg)) == (int)NULL)
					OPT_MISSING(argv[0], optopt);
				else
					if (num_thrd < 1)
                    {
						fprintf(stdout, "WARNING: bad arg. Using default\n");
                        num_of_procs = 1; /* setting default = uni processor */
                    }
				break;
            case 't':
				if ((num_thrd = atoi(optarg)) == (int)NULL)
					OPT_MISSING(argv[0], optopt);
				else
					if (num_thrd < 1)
                    {
						fprintf(stdout, "WARNING: bad arg. Using default\n");
                        num_thrd = MAXT;
                    }
				break;
            case 'v':
                verbose = 1;
                break;
            default :
				usage(argv[0]);
				break;
        }
    }

    status = malloc(sizeof(task_str_t));

    for(iter = 0; iter < num_of_procs; iter++)
    {
        /* create num_thrd number of threads per processor that is available. */
        for (thrd_ndx = 0; thrd_ndx<num_thrd; thrd_ndx++)
        {
            child_str = malloc(sizeof(task_str_t));

            if (pthread_create(&thid[thrd_ndx], NULL, thread_func, child_str))
            {
                perror("main(): pthread_create()");
                exit(-1);
            }
            if (verbose)
                fprintf(stdout, "Created thread[%d]\n", thrd_ndx);
            usleep(9);
            sched_yield();
        }

        /* wait for the children to terminate */
        for (thrd_ndx = 0; thrd_ndx<num_thrd; thrd_ndx++)
        {
            if (pthread_join(thid[thrd_ndx], (void **)&status))
            {
                perror("main(): pthread_join()");
                exit(-1);
            }
            else
            {
                if ((int)status == -1)
                {
                    fprintf(stderr, 
                             "thread [%d] - process exited with errors %d\n",
                             thrd_ndx, WEXITSTATUS(status[0]));
                exit(-1);
                }
        
                child_str = status;
                fprintf(stdout, "Pid = %d\n Exp prio = %d\n Set prio = %d "
                "Processor Num = %d\n",  
                    child_str->procs_id,
                    child_str->exp_prio,
                    child_str->act_prio,
                    child_str->proc_num);

                exp_prio[thrd_ndx] = chld_args[0];
                act_prio[thrd_ndx] = chld_args[1];
                proc_id[thrd_ndx]  = chld_args[2];
                gen_pid[thrd_ndx]  = chld_args[3];
            }
            usleep(10);
        }

        for (proc_num = 0; proc_num < num_of_procs; proc_num++)
        {
            for (pid_ndx = 0; pid_ndx < num_thrd; pid_ndx++)
            {
                if (proc_id[pid_ndx] == proc_num)
                    fprintf(stdout,"pid = %d proc = %d exp_p = %d act_p = %d\n",
                        gen_pid[pid_ndx], proc_id[pid_ndx], exp_prio[pid_ndx],
                        act_prio[pid_ndx]);
            }
        } 
                
    }
    free(status);
    exit(0);
}
