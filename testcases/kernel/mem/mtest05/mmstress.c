/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/******************************************************************************/
/* 									      */
/* Apr-13-2001	Created: Manoj Iyer, IBM Austin. 			      */
/*		These tests are adapted from AIX vmm FVT tests.		      */
/* 									      */
/* Oct-24-2001  Modified. 						      */
/*		- freed buffers that were allocated.			      */
/*		- closed removed files. This will remove the disk full error  */
/*		- use pthread_exit in case of theads instead of return. This  */
/*		  was really bad to use return!                               */
/*		- created usage function.                                     */
/*		- pthread_join checks for thread exit status reported by      */
/*		  pthread_exit()                                              */
/*								              */
/* Purpose:	Performing General Stress with Race conditions        	      */
/* 									      */
/******************************************************************************/

/* GLOBAL INCLUDE FILES							      */
#include <stdio.h>	/* standard C input/output routines		      */
#include <pthread.h>
#include <sys/types.h>  /* definitions required for open routine              */
#include <sys/stat.h>	/* definitions required for open routine              */
#include <fcntl.h>	/* definitions required for open routine              */
#include <unistd.h>	/* required by mmap routine		              */
#include <sys/mman.h>   /* definitions required by mmap routine		      */
#include <sys/wait.h>	/* definitiion for wait routine			      */
#include <sys/time.h>   /* definitions for the interval timer		      */
#include <pthread.h>    /* definitions for creating threads                   */
#include <signal.h>     /* definitions for signals - required for SIGALRM     */
#include <errno.h>	/* definitions for errors		              */

/* GLOBAL DEFINES							      */
#define READ_FAULT      0
#define WRITE_FAULT     1
#define COW_FAULT       2
#define NUMTHREAD	32   /* Assume we can have a maximum of 32 CPUS	      */
#define NUMPAGES	2000 /* no real reason for choosing this number       */
#define INFOPAGES	17   /* number of pages described by the table        */
#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif
#define FAILED		(-1)
#define SUCCESS		0
#define OPT_MISSING(prog, opt) do {\
			fprintf(stderr, "%s: option -%c ", prog, opt); \
			fprintf(stderr, "requires and argument\n");\
			    } while(0)

#define PTHREAD_EXIT(val)      do {\
			exit_val = val; \
		        pthread_exit((void *)&exit_val); \
                                  } while (0)

#define MAXTEST		7

/* GLOBAL VARIABLES							      */
typedef struct {
	int	status;	     /* status of the operation - FAILED or SUCCESS   */
	caddr_t mapaddr;     /* address at which the file is mapped	      */
} RETINFO_t;

static   int  wait_thread; /* used to wake up sleeping threads                */
static   int  verbose_print = FALSE; /* print more test information           */


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
/* Function:    usage                                                         */
/*                                                                            */
/* Description: prints the usage function.                                    */
/*                                                                            */
/* Input:	char *progname - name of this executable.                     */
/*                                                                            */
/* Return:      exit 1						              */
/*                                                                            */
/******************************************************************************/
static void 
usage(char *progname)		/* name of this program                       */
{
    fprintf(stderr, "usage:%s -n test -t time -v [-V]\n", *progname);
    fprintf(stderr, "\t-n the test number, if no test number\n"
		    "\t   is specified all the tests will be run\n");
    fprintf(stderr, "\t-t specify the time in hours\n");
    fprintf(stderr, "\t-v verbose output\n");
    fprintf(stderr, "\t-V program version\n");
    exit(1);
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
set_timer(int run_time)         /* period for which test is intended to run   */{
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


/******************************************************************************/
/*									      */
/* Function:	thread_fault						      */
/*									      */
/* Description: Executes as a thread function and accesses the memory pages   */
/*	        depending on the fault_type to be generated. This function can*/
/*		cause READ fault, WRITE fault, COW fault.		      */
/* 									      */
/* Input:	void *args - argments passed to the exec routine by           */
/*	 	pthread_create()				              */
/*			local_args[0] - the thread number		      */
/*			local_args[1] - map address	 		      */
/*			local_args[2] - page size			      */
/*			local_args[3] - fault type			      */
/*								              */
/* Output:	NONE							      */
/******************************************************************************/
static void *
thread_fault(void *args)         /* pointer to the arguments passed to routine*/
{
    long   *local_args   = args; /* local pointer to list of arguments	      */
    int     pgnum_ndx    = 0;	 /* index to the number of pages	      */
    caddr_t start_addr		 /* start address of the page		      */
		         = (caddr_t)(local_args[1] + (int)local_args[0] 
		 	   * (NUMPAGES/NUMTHREAD) * local_args[2]);
    char    read_from_addr;	 /* address to which read from page is done   */
    char    write_to_addr[] = {'a'}; /* character to be writen to the page    */
    volatile int exit_val = 0;   /* exit value of the pthread. 0 - success    */

    while (wait_thread);

    for (; pgnum_ndx < (NUMPAGES/NUMTHREAD); pgnum_ndx++)
    {
	/* if the fault to be generated is READ_FAULT, read from the page     */
	/* else write a character to the page.				      */
        ((int)local_args[3] == READ_FAULT) ? (read_from_addr = *start_addr)
			         : (*start_addr = write_to_addr[0]);
         start_addr += local_args[2]; 
         if (verbose_print)
             fprintf(stdout, "thread_fault(): generating fault type %d" 
		    " @page adress %#lx\n", (int)local_args[3], 
		     start_addr);
	 fflush(NULL);
    }
    PTHREAD_EXIT(0);
}


/******************************************************************************/
/*							                      */
/* Function:	thread_mmap						      */
/* 								              */
/* Description:	perfom mmap operations over the base mmap that was performed  */
/*              by the controlling process.			              */
/*								              */
/******************************************************************************/
static void *
thread_mmap(void *args)		/* pointer to the arguments passed in         */
{
    long *local_args = args;	/* local pointer to list of arguments	      */
    int   page_ndx;	        /* index to the page		              */
    int   num_pages;	        /* number of pages			      */
    off_t map_offset;		/* offset from the base addr to start mmap    */
    char *layout     = 		/* local pointer to memory layout	      */
		       (char *)local_args[4];
    volatile int exit_val = 0;  /* exit value of the pthread. 0 - success    */

    while (wait_thread);	/* when wait set to true - loop		      */

    while (scanf(layout, "%lx%lx", &page_ndx, &num_pages) == 2)
    {
        map_offset = num_pages * local_args[2];
	if (mmap((caddr_t)(local_args[1] + map_offset), map_offset, 
		PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, (int)local_args[5],
		map_offset) == (caddr_t)-1)
        {
	    perror("thread_mmap(): mmap()");
	    PTHREAD_EXIT(1);
        }
        layout += 2;
    }
    PTHREAD_EXIT(0);
}


/******************************************************************************/
/*							                      */
/* Function:	remove_tmpfiles						      */
/* 								              */
/* Description:	remove temporary files that were created by the tests.        */
/*              						              */
/* Input:	char *filename - name of the file to be removed.	      */
/*              						              */
/* Output:	None							      */
/*								              */
/******************************************************************************/
static int
remove_files(char *filename)	/* name of the file that is to be removed     */
{
    if (strcmp(filename, "NULL") || strcmp(filename, "/dev/zero"))
        if (unlink(filename))
        {
            perror("map_and_thread(): ulink()");
            return FAILED;
        }
	else
	{
            if (verbose_print)
	        fprintf(stdout, "file %s removed\n", filename);
	    return SUCCESS;
        }
}


/******************************************************************************/
/*									      */
/* Function:	map_and_thread						      */
/*									      */
/* Description:	Creates mappings with the required properties, of MAP_PRIVATE */
/*		MAP_SHARED and of PROT_RED / PROT_READ|PROT_WRITE.	      */
/*		Create threads and execute a routine that will generate the   */
/*		desired fault condition, viz, read, write and cow fault.      */
/*									      */
/* Input:	char *tmpfile - name of temporary file that is created        */
/*		int   fault_type - type of fault that is to be generated.     */
/*									      */
/* Output:	NONE							      */
/*									      */
/******************************************************************************/
RETINFO_t *
map_and_thread(char  *tmpfile,	      /* name of temporary file to be created */
	       void  *(*exec_func)(void *),/* thread function to execute      */
	       int    fault_type,     /* type of fault to be generated	      */
	       int    num_thread,     /* number of threads to create	      */
	       RETINFO_t *retinfo)    /* return map address and oper status   */
	       
{
    int  fd;			/* file descriptor of the file created	      */
    int  page_ndx;	 	/* number of pages to write to the temp file  */
    int  thrd_ndx = 0;	 	/* index to the number of threads created     */
    int  map_type;	        /* specifies the type of the mapped object    */
    int  *th_status;            /* status of the thread when it is finished   */
    long th_args[NUMTHREAD];    /* argument list passed to  thread_fault()    */
    char *empty_buf;		/* empty buffer used to fill temp file	      */
    long pagesize 		/* contains page size at runtime	      */
	 = sysconf(_SC_PAGESIZE);		
    static pthread_t pthread_ids[NUMTHREAD];
				/* contains ids of the threads created        */
    caddr_t map_addr;		/* address where the file is mapped	      */
				
    /* Create a file with permissions 0666, and open it with RDRW perms	*/
    /* if the name is not a NULL         			        */
    if (strcmp(tmpfile, "NULL"))
    {
        if ((fd = open(tmpfile, O_RDWR|O_CREAT, S_IRWXO|S_IRWXU|S_IRWXG)) 
		== -1 )
        {
	    perror("map_and_thread(): open()");
	    close(fd);
	    fflush(NULL);
	    retinfo->status = FAILED;
	    return retinfo;
        }

        /* Write pagesize * NUMPAGES bytes to the file */
        empty_buf = (char *)malloc(pagesize*NUMPAGES);
        if (write(fd, empty_buf, pagesize*NUMPAGES) != (pagesize*NUMPAGES))
        {
	    perror("map_and_thread(): write()");
	    free(empty_buf);
	    fflush(NULL);
            remove_files(tmpfile);
            close(fd);
	    retinfo->status = FAILED;
	    return retinfo;
        }
	    map_type = (fault_type == COW_FAULT) ? MAP_PRIVATE : MAP_SHARED; 

        /* Map the file, if the required fault type is COW_FAULT map the file */
        /* private, else map the file shared. if READ_FAULT is required to be */
        /* generated map the file with read protection else map with read -   */
        /* write protection. 						      */
    
        if ((map_addr = (caddr_t)mmap(0, pagesize*NUMPAGES, 
		        ((fault_type == READ_FAULT) ? PROT_READ : 
						  PROT_READ|PROT_WRITE),
		                        map_type, fd, 0)) == MAP_FAILED)
        {
            perror("map_and_thread(): mmap()");
	    free(empty_buf);
	    fflush(NULL);
            remove_files(tmpfile);
            close(fd);
	    retinfo->status = FAILED;
	    return retinfo;
        }
        else
	{
	    retinfo->mapaddr = map_addr;
            if (verbose_print)
    	        fprintf(stdout, 
		  "map_and_thread(): mmap success, address = %#lx\n", map_addr);
	    fflush(NULL);
	}
    }

    /* As long as wait is set to TRUE, the thread that will be created will */
    /* loop in its exec routine */
    wait_thread = TRUE;

    /* Create a few threads, ideally number of threads equals number of CPU'S */
    /* so that we can assume that each thread will run on a single CPU in     */
    /* of SMP machines. Currently we will create NR_CPUS number of threads.   */
    th_args[1] = (long)map_addr;
    th_args[2] = pagesize;
    th_args[3] = fault_type;
    do
    {
	th_args[0] = thrd_ndx;
        th_args[4] = (long)0;
	
        if (pthread_create(&pthread_ids[thrd_ndx++], NULL, exec_func,
		       (void *)&th_args))
        {
	    perror("map_and_thread(): pthread_create()");
	    free(empty_buf);
	    fflush(NULL);
            remove_files(tmpfile);
            close(fd);
	    retinfo->status = FAILED;
	    return retinfo;
        }
    } while (thrd_ndx < num_thread);

    if (verbose_print)
        fprintf(stdout, "map_and_thread(): pthread_create() success\n");
    wait_thread = FALSE; 
    
    /* suspend the execution of the calling thread till the execution of the  */
    /* other thread has been terminated.				      */
    for (thrd_ndx = 0; thrd_ndx < NUMTHREAD; thrd_ndx++)
    {
        if (pthread_join(pthread_ids[thrd_ndx],(void **)th_status))
        {
            perror("map_and_thread(): pthread_join()");
	    free(empty_buf);
            fflush(NULL);
            remove_files(tmpfile);
            close(fd);
	    retinfo->status = FAILED;
	    return retinfo;
        }
        else
        {
            if ((int)*th_status == 1)
            {
                fprintf(stderr,
                        "thread [%d] - process exited with errors\n",
			    pthread_ids[thrd_ndx]);
                free(empty_buf);                          
                remove_files(tmpfile);
                close(fd);
                exit(1);
            }
        }   
    }
    
    /* remove the temporary file that was created. - clean up                 */
    /* but dont try to remove special files.			              */
    if (!remove_files(tmpfile))
        {
	    free(empty_buf);
	    retinfo->status = FAILED;
	    return retinfo;
        }
    free(empty_buf);
    close(fd);
    retinfo->status = SUCCESS;
    return retinfo;
}


/******************************************************************************/
/* 								              */
/* Test:	Test case tests the race condition between simultaneous read  */
/* 		faults in the same address space.			      */
/*									      */
/* Description:	map a file into memory, create threads and execute a thread   */
/*		function that will cause read faults by simulteniously reading*/
/*		fom this memory space.				              */
/* 								              */
/******************************************************************************/
static int
test1() 
{
    RETINFO_t retval;	/* contains info relating to test status	      */

    printf("\ttest1: Test case tests the race condition between\n"
	   "\t\tsimultaneous read faults in the same address space.\n");
    map_and_thread("./tmp.file.1", thread_fault, READ_FAULT, 32, &retval);
    return (retval.status);
}


/******************************************************************************/
/* 								              */
/* Test:	Test case tests the race condition between simultaneous write */
/* 		faults in the same address space.			      */
/*									      */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause write faults by simulteniously       */
/*              writing to this memory space.                                 */
/* 								              */
/******************************************************************************/
static int
test2() 
{
    RETINFO_t retval;	/* contains test stats information 		      */

    printf("\ttest2: Test case tests the race condition between\n"
	   "\t\tsimultaneous write faults in the same address space.\n");
    map_and_thread("./tmp.file.2", thread_fault, WRITE_FAULT, 32, &retval);
    return (retval.status);
}


/******************************************************************************/
/* 								              */
/* Test:	Test case tests the race condition between simultaneous COW   */
/* 		faults in the same address space.			      */
/*									      */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause COW faults by simulteniously         */
/*              writing to this memory space.                                 */
/* 								              */
/******************************************************************************/
static int
test3() 
{
    RETINFO_t retval;	/* contains test stats information		      */
 
    printf("\ttest3: Test case tests the race condition between\n"
	   "\t\tsimultaneous COW faults in the same address space.\n");
    map_and_thread("./tmp.file.3", thread_fault, COW_FAULT, 32, &retval);
    return (retval.status);
}


/******************************************************************************/
/* 								              */
/* Test:	Test case tests the race condition between simultaneous READ  */
/* 		faults in the same address space. File maped is /dev/zero     */
/*									      */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause READ faults by simulteniously        */
/*              writing to this memory space.                                 */
/* 								              */
/******************************************************************************/
static int
test4() 
{
    RETINFO_t retval; 	/* contains test status information		      */

    printf("\ttest4: Test case tests the race condition between\n"
	   "\t\tsimultaneous READ faults in the same address space.\n"
	   "\t\tThe file maped is /dev/zero\n");
    map_and_thread("/dev/zero", thread_fault, COW_FAULT, 32, &retval);
    return (retval.status);
}


/******************************************************************************/
/* 								              */
/* Test:	Test case tests the race condition between simultaneous       */
/* 		fork - exit faults in the same address space.                 */
/*									      */
/* Description: Initilize large data in the parent process, fork a child and  */
/*              and the parent waits for the child to complete execution.     */
/*              						              */
/* 								              */
/******************************************************************************/
static int
test5() 
{
    int    fork_ndx = 0;/* index to the number of processes forked            */
    pid_t  pid;		/* process id, returned by fork system call.	      */
    int    wait_status; /* if status is not NULL store status information     */

    printf("\ttest5: Test case tests the race condition between\n"
 	   "\t\tsimultaneous fork - exit faults in the same address space.\n");

    /* increment the  program's  data  space  by 200*1024 (200K) bytes        */
    if (sbrk(200*1024) == (caddr_t)-1)
    {
        perror("test5(): sbrk()");
	fflush(NULL);
        return FAILED;
    }
   
    /* fork NUMTHREAD number of processes, assumption is on SMP each will get */
    /* a separate CPU if NRCPUS = NUMTHREAD. The child does nothing; exits    */
    /* imideately, parent waits for child to complete execution.              */
    do
    {
        if (!(pid = fork()))
	    exit (0);
        else
            if (pid != -1)
		wait(&wait_status);
        
    } while (fork_ndx++ < NUMTHREAD);
    
    if (sbrk(-200*1024) == (caddr_t)-1)
    {
        printf("test5(): rollback sbrk failed\n");
	fflush(NULL);
        perror("test5(): sbrk()");
        fflush(NULL);
        return FAILED;
    }
    return SUCCESS;
}


/******************************************************************************/
/* 								              */
/* Test:	Test case tests the race condition between simultaneous       */
/* 		fork - exec - exit faults in the same address space.          */
/*									      */
/* Description: Initilize large data in the parent process, fork a child and  */
/*              and the parent waits for the child to complete execution. The */
/*		child program execs a dummy program.			      */
/*              						              */
/* 								              */
/******************************************************************************/
static int
test6()
{
    int    fork_ndx = 0;/* index to the number of processes forked            */
    pid_t  pid;         /* process id, returned by fork system call.          */
    int    wait_status; /* if status is not NULL store status information     */
    char  *argv_init[2] =  /* parameter required for dummy fiunction to execv */
                      {"arg1", NULL};

    printf("\ttest6: Test case tests the race condition between\n"
           "\t\tsimultaneous fork -exec - exit faults in the same\n"
	   "\t\taddress space.\n");

    /* increment the  program's  data  space  by 200*1024 (200K) bytes        */
    if (sbrk(200*1024) == (caddr_t)-1)
    {
        perror("test5(): sbrk()");
        fflush(NULL);
        return FAILED;
    }

    /* fork NUMTHREAD number of processes, assumption is on SMP each will get */
    /* a separate CPU if NRCPUS = NUMTHREAD. The child execs a dummy program  */
    /*  and parent waits for child to complete execution.                     */
    do
    {
        if (!(pid = fork()))
	{
            if (execv("dummy", argv_init) == -1)
            {
	        perror("test6(): execv()");
		fflush(NULL);
		return FAILED;
	    }
        }
        else
            if (pid != -1)
                wait(&wait_status);

    } while (fork_ndx++ < NUMTHREAD);
   
    if (sbrk(-200*1024) == (caddr_t)-1)
    {
        printf("test5(): rollback sbrk failed\n");
        fflush(NULL);
        perror("test5(): sbrk()");
        fflush(NULL);
        return FAILED;
    }
    return SUCCESS;
}

    
/******************************************************************************/
/* 								              */
/* Function:	main						              */
/* 								              */
/* Desciption:	This is the main function of mmstress program. It processes   */
/*		all command line options and flags and runs the tests.        */
/*									      */
/* Input:	argc - number of command line parameters		      */
/*		argv - pointer to the array of the command line parameters.   */
/*									      */
/* Output:	none							      */
/*								              */
/******************************************************************************/
main(int   argc,    /* number of command line parameters		      */
     char **argv)   /* pointer to the array of the command line parameters.   */
{
    extern char *optarg;/* getopt() function global variables	      */
    extern int   optind;
    extern int   opterr;
    extern int   optopt;

    static char *version_info = "mmstress V1.00 04/17/2001";

    int (*(test_ptr)[])() =	/* pointer to the array of test names         */
			    {NULL, test1, test2, test3, test4, test5, test6};
    int   ch;			/* command line flag character		      */
    int   test_num  = 0;	/* test number that is to be run              */
    int   test_time = 0;	/* duration for which test is to be run       */
    int   sig_ndx;              /* index into signal handler structure.       */
    char *prog_name = argv[0]; 	/* name of this program			      */
    int   rc = 0;		/* return value from the tests.	0 - success   */
    int   version_flag = FALSE; /* printf the program version		      */
    struct sigaction sigptr;    /* set up signal, for interval timer          */

    static struct signal_info
    {
        int  signum;    /* signal number that hasto be handled                */        char *signame;  /* name of the signal to be handled.                  */    } sig_info[] =
                   {
                        SIGHUP,"SIGHUP",
                        SIGINT,"SIGINT",
                        SIGQUIT,"SIGQUIT",
                        SIGABRT,"SIGABRT",
                        SIGBUS,"SIGBUS",
                        SIGSEGV,"SIGSEGV",
                        SIGALRM, "SIGALRM",
                        SIGUSR1,"SIGUSR1",
                        SIGUSR2,"SIGUSR2",
                        -1,     "ENDSIG"
                   };
    
    optarg = NULL;
    opterr = 0;

    if (argc < 2)
    {
        usage(argv);
    }
    if (*argv[1] == '-')
    {
        while ((ch = getopt(argc, argv, "n:t:vV")) != -1)
	{
	    switch(ch)
	    {
		case 'n': if (optarg) 
		              test_num = atoi(optarg);
			  else
		              OPT_MISSING(prog_name, optopt);
	                  break;
		case 't': if (optarg)
			      printf("Test is scheduld to run for %d hours\n",
			           test_time = atoi(optarg));
		          else
			      OPT_MISSING(prog_name, optopt); 	          
			  break;
                case 'v': verbose_print = TRUE;
		          break;
		case 'V': if (!version_flag)
		          {
			      version_flag = TRUE;
			      fprintf(stderr, "%s: %s\n", prog_name,
			              version_info);
			  }
		          break;
                case '?': if (argc < optind)
			      OPT_MISSING(prog_name, optopt);
			  else
			      fprintf(stderr, 
			         "%s: unknown option - %c ignored\n", prog_name,
			         optopt);
		          break;
		default: fprintf(stderr, "%s: getopt() failed!!!\n", prog_name);
			 exit(2);
            }
        }
    }

    /* duration for which the tests have to be run. test_time is converted to */
    /* corresponding seconds and the tests are run as long as the current time*/
    /* is less than the required time and test are successul (ie rc = 0)      */
    //test_time = test_time*60*60 + time(NULL);
    set_timer(test_time);

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
            perror( "main(): sigaction()" );
            fprintf(stderr, "could not set handler for SIGALRM, errno = %d\n",
                    errno);
            exit(-1);
        }
    }

    do
    {
        if(!test_num)
        {
            int test_ndx;
	    for (test_ndx = 1; test_ndx <= MAXTEST -1; test_ndx++)
	        rc = test_ptr[test_ndx]();
        }
        else
        {
            rc = (test_num > MAXTEST) ? fprintf(stderr, 
		  "Invalid test number, must be in range [1 - %d]\n", MAXTEST): 
	           test_ptr[test_num]();
	}
        if (rc != SUCCESS)
            exit(rc);
    }
    while (TRUE);
    exit(rc);
}
