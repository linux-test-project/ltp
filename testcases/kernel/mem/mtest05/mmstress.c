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
/*                                                                            */
/* File:         mmstress.c                                                   */
/*                                                                            */
/* Description:  This is a test program that performs general stress with     */
/*               memory race conditions. It contains seven testcases that     */
/*               will test race conditions between simultaneous read fault,   */
/*               write fault, copy on write (COW) fault e.t.c.                */
/*               This testcase is intended to execute on the Linux operating  */
/*               system and can be easily ported to work on other operating   */
/*               systems as well.                                             */
/*                                                                            */
/* Usage:        mmstress -h -n TEST NUMBER -p NPAGES -t EXECUTION TIME -v -V */
/*                        -h                - Help                            */
/*                        -n TEST NUMBER    - Execute a particular testcase   */
/*                        -p NPAGES         - Use NPAGES pages for tests    */
/*                        -t EXECUTION TIME - Execute test for a certain time */
/*                        -v                - Verbose output                  */
/*                        -V                - Version of this program         */
/*                                                                            */
/* Author:       Manoj Iyer - manjo@mail.utexas.edu                           */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Apr-13-2001    Created: Manoj Iyer, IBM Austin.                            */
/*        These tests are adapted from AIX vmm FVT tests.                     */
/*                                                                            */
/* Oct-24-2001  Modified.                                                     */
/*        - freed buffers that were allocated.                                */
/*        - closed removed files. This will remove the disk full error        */
/*        - use pthread_exit in case of theads instead of return. This        */
/*          was really bad to use return!                                     */
/*        - created usage function.                                           */
/*        - pthread_join checks for thread exit status reported by            */
/*          pthread_exit()                                                    */
/*                                                                            */
/* Oct-25-2001  Modified.                                                     */
/*        - Fixed bug in usage()                                              */
/*        - malloc'ed pointer for pthread return value.                       */
/*        - changed scheme. If no options are specified, all the tests        */
/*          will be run once.                                                 */
/*                                                                            */
/* Nov-02-2001  Modified - Paul Larson                                        */
/*        - Added sched_yield to thread_fault to fix hang                     */
/*        - Removed thread_mmap                                               */
/*                                                                            */
/* Nov-09-2001  Modified - Manoj Iyer                                         */
/*        - Removed compile warnings.                                         */
/*        - Added missing header file. #include <stdlib.h>                    */
/*                                                                            */
/* Oct-28-2003  Modified - Manoj Iyer                                         */
/*        - missing parenthesis added.                                        */
/*        - formatting changes.                                               */
/*        - increased NUMPAGES to 9999.                                       */
/*                                                                            */
/* Jan-30-2003  Modified - Gary Williams                                      */
/*        - fixed a race condition between the two threads                    */
/*        - made it so if any of the testcases fail the test will fail        */
/*        - fixed so status of child in test 6 is used to determine result    */
/*        - fixed the use of the remove_files function in a conditional       */
/*                                                                            */
/******************************************************************************/

/* GLOBAL INCLUDE FILES                                                       */
#include <stdio.h>    /* standard C input/output routines                     */
#include <sys/types.h>/* definitions for open()                               */
#include <sys/stat.h> /* definitions for open()                               */
#include <fcntl.h>    /* definition of open()                                 */
#include <unistd.h>   /* declaration of close()                               */
#include <sys/mman.h> /* declaration of mmap()                                */
#include <sys/wait.h> /* declaration for wait routine                         */
#include <sys/time.h> /* definitions for the interval timer                   */
#include <pthread.h>  /* declaration of pthread functions                     */
#include <signal.h>   /* definitions for signals - required for SIGALRM       */
#include <errno.h>    /* definitions for errors                               */
#include <stdlib.h>   /* declaration for malloc                               */
#include <string.h>   /* declaration for memset                               */
#include <sched.h>    /* declaration of sched_yield()                         */
#include <stdint.h>

#include "test.h"

/* GLOBAL DEFINES                                                             */
#define SIGENDSIG    -1   /* end of signal marker                             */
#define THNUM        0    /* array element pointing to number of threads      */
#define MAPADDR      1    /* array element pointing to map address            */
#define PAGESIZ      2    /* array element pointing to page size              */
#define FLTIPE       3    /* array element pointing to fault type             */
#define READ_FAULT   0    /* instructs routine to simulate read fault         */
#define WRITE_FAULT  1    /* instructs routine to simulate write fault        */
#define COW_FAULT    2    /* instructs routine to simulate copy-on-write fault*/
#define NUMTHREAD    32   /* number of threads to spawn default to 32         */
#define NUMPAGES     9999 /* default (random) value of number of pages        */
#ifndef TRUE
#define TRUE         1
#endif
#ifndef FALSE
#define FALSE        0
#endif
#define FAILED       (-1) /* return status for all funcs indicating failure   */
#define SUCCESS      0    /* return status for all routines indicating success*/
                          /* prints error if no argument passed               */
#define OPT_MISSING(prog, opt) do {                        \
            fprintf(stderr, "%s: option -%c ", prog, opt); \
            fprintf(stderr, "requires an argument\n");    \
                } while (0)

                          /* exit thread macro                                */
#define PTHREAD_EXIT(val)      do {              \
            exit_val = val;                      \
                pthread_exit((void *)exit_val); \
                                  } while (0)
#define MAXTEST      6   /* total number of testcase in this program          */
#define BRKSZ        512*1024 /* program data space allocation value          */

/* GLOBAL VARIABLES                                                           */
typedef struct {      /* structure returned by map_and_thread()               */
    int    status;    /* status of the operation - FAILED or SUCCESS          */
    caddr_t mapaddr;  /* address at which the file is mapped                  */
} RETINFO_t;

static   int  wait_thread = 0;       /* used to wake up sleeping threads      */
static   int  thread_begin = 0;      /* used to coordinate threads            */
static   int  verbose_print = FALSE; /* print more test information           */

static   int    pages_num = NUMPAGES; /* number of pages to use for tests     */

char *TCID = "mmstress";
int TST_TOTAL = 6;

/******************************************************************************/
/*                                                                            */
/* Function:    sig_handler                                                   */
/*                                                                            */
/* Description: handle SIGALRM raised by set_timer(), SIGALRM is raised when  */
/*              the timer expires. If any other signal is received exit the   */
/*              test.                                                         */
/*                                                                            */
/* Input:       signal - signal number, intrested in SIGALRM!                 */
/*                                                                            */
/* Return:      exit -1 if unexpected signal is received                      */
/*              exit 0 if SIGALRM is received                                 */
/*                                                                            */
/* Synopsis:    void signal_handler(int signal);                              */
/*                                                                            */
/******************************************************************************/
static void
sig_handler(int signal) /* signal number, set to handle SIGALRM               */
{
    if (signal != SIGALRM)
    {
        fprintf(stderr, "sig_handlder(): unexpected signal caught [%d]\n",
            signal);
        exit(-1);
    }
    else
	tst_resm(TPASS, "Test ended, success");
    exit(0);
}

/******************************************************************************/
/*                                                                            */
/* Function:    usage                                                         */
/*                                                                            */
/* Description: prints the usage function.                                    */
/*                                                                            */
/* Input:       char *progname - name of this executable.                     */
/*                                                                            */
/* Return:      exit 1                                                        */
/*                                                                            */
/* Synopsis:    void usage(char *progname);                                   */
/*                                                                            */
/******************************************************************************/
static void
usage(char *progname)        /* name of this program                       */
{
    fprintf(stderr, "usage:%s -h -n test -t time -v [-V]\n", progname);
    fprintf(stderr, "\t-h displays all options\n");
    fprintf(stderr, "\t-n test number, if no test number\n"
            "\t   is specified, all the tests will be run\n");
    fprintf(stderr, "\t-p specify the number of pages to\n"
            "\t   use for allocation\n");
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
/* Synopsis:    void set_timer(int run_time);                                 */
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
/*                                                                            */
/* Function:    thread_fault                                                  */
/*                                                                            */
/* Description: Executes as a thread function and accesses the memory pages   */
/*              depending on the fault_type to be generated. This function    */
/*              can cause READ fault, WRITE fault, COW fault.                 */
/*                                                                            */
/* Input:       void *args - argments passed to the exec routine by           */
/*              pthread_create()                                              */
/*                                                                            */
/* Return:      NONE                                                          */
/*                                                                            */
/* Synopsis:    void *thread_fault(void *args);                               */
/*                                                                            */
/******************************************************************************/
static void *
thread_fault(void *args)         /* pointer to the arguments passed to routine*/
{
    long   *local_args   = args; /* local pointer to list of arguments        */
                                 /* local_args[THNUM]   - the thread number   */
                                 /* local_args[MAPADDR] - map address         */
                                 /* local_args[PAGESIZ] - page size           */
                                 /* local_args[FLTIPE]  - fault type          */
    int     pgnum_ndx    = 0;    /* index to the number of pages              */
    caddr_t start_addr           /* start address of the page                 */
                         = (caddr_t)(local_args[MAPADDR]
                                     + (int)local_args[THNUM]
                                     * (pages_num/NUMTHREAD)
                                     * local_args[PAGESIZ]);
    char    read_from_addr = 0;  /* address to which read from page is done   */
    char    write_to_addr[] = {'a'}; /* character to be writen to the page    */
    uintptr_t exit_val = 0;   /* exit value of the pthread. 0 - success    */

    /*************************************************************/
    /*   The way it was, args could be overwritten by subsequent uses
     *   of it before this routine had a chance to use the data.
     *   This flag stops the overwrite until this routine gets to
     *   here.  At this point, it is done initializing and it is
     *   safe for the parent thread to continue (which will change
     *   args).
     */
    thread_begin = FALSE;

    while (wait_thread)
        sched_yield();

    for (; pgnum_ndx < (pages_num/NUMTHREAD); pgnum_ndx++)
    {
        /* if the fault to be generated is READ_FAULT, read from the page     */
        /* else write a character to the page.                                */
        ((int)local_args[3] == READ_FAULT) ? (read_from_addr = *start_addr)
                                           : (*start_addr = write_to_addr[0]);
        start_addr += local_args[PAGESIZ];
        if (verbose_print)
	    tst_resm(TINFO, "thread_fault(): generating fault type %ld"
                            " @page address %p", local_args[3], start_addr);
        fflush(NULL);
    }
    PTHREAD_EXIT(0);
}

/******************************************************************************/
/*                                                                            */
/* Function:    remove_tmpfiles                                               */
/*                                                                            */
/* Description: remove temporary files that were created by the tests.        */
/*                                                                            */
/* Input:       char *filename - name of the file to be removed.              */
/*                                                                            */
/* Return:      FAILED  - on failure                                          */
/*              SUCCESS - on success                                          */
/*                                                                            */
/* Synopsis:    int remove_files(char *filename);                             */
/*                                                                            */
/******************************************************************************/
static int
remove_files(char *filename, char * addr)    /* name of the file that is to be removed     */
{
    if (addr)
		     if (munmap(addr, sysconf(_SC_PAGESIZE)*pages_num) < 0) {
		 		     perror("map_and_thread(): munmap()");
		 		     return FAILED;
		     }
    if (strcmp(filename, "NULL") && strcmp(filename, "/dev/zero"))
    {
        if (unlink(filename))
        {
            perror("map_and_thread(): ulink()");
            return FAILED;
        }
    }
    else
    {
        if (verbose_print)
            tst_resm(TINFO, "file %s removed", filename);

    }
    return SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* Function:    map_and_thread                                                */
/*                                                                            */
/* Description: Creates mappings with the required properties, of MAP_PRIVATE */
/*              MAP_SHARED and of PROT_RED / PROT_READ|PROT_WRITE.            */
/*              Create threads and execute a routine that will generate the   */
/*              desired fault condition, viz, read, write and cow fault.      */
/*                                                                            */
/* Input:       char *tmpfile - name of temporary file that is created        */
/*              int   fault_type - type of fault that is to be generated.     */
/*                                                                            */
/* Return:    RETINFO_t * - pointer to RETINFO_t structure                    */
/*                                                                            */
/* Synopsis:  RETINFO_t *map_and_thread(                                      */
/*                        char  *tmpfile,                                     */
/*                        void  *(*exec_func)(void *),                        */
/*                        int    fault_type,                                  */
/*                        int    num_thread,                                  */
/*                        RETINFO_t *retinfo);                                */
/*                                                                            */
/******************************************************************************/
RETINFO_t *
map_and_thread(
    char  *tmpfile,              /* name of temporary file to be created      */
    void  *(*exec_func)(void *), /* thread function to execute                */
    int    fault_type,           /* type of fault to be generated             */
    int    num_thread,           /* number of threads to create               */
    RETINFO_t *retinfo)          /* return map address and oper status        */

{
    int  fd         = 0;         /* file descriptor of the file created       */
    int  thrd_ndx   = 0;         /* index to the number of threads created    */
    int  map_type   = 0;         /* specifies the type of the mapped object   */
    int  *th_status = 0;         /* status of the thread when it is finished  */
    long th_args[5];     	/* argument list passed to  thread_fault()   */
    char *empty_buf = NULL;      /* empty buffer used to fill temp file       */
    long pagesize                /* contains page size at runtime             */
                    = sysconf(_SC_PAGESIZE);
    static pthread_t pthread_ids[NUMTHREAD];
                                 /* contains ids of the threads created       */
    caddr_t map_addr = NULL;     /* address where the file is mapped          */

    /* Create a file with permissions 0666, and open it with RDRW perms       */
    /* if the name is not a NULL                                              */

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

        /* Write pagesize * pages_num bytes to the file */
        empty_buf = (char *)malloc(pagesize*pages_num);
        if (write(fd, empty_buf, pagesize*pages_num) != (pagesize*pages_num))
        {
            perror("map_and_thread(): write()");
            free(empty_buf);
            fflush(NULL);
            remove_files(tmpfile, NULL);
            close(fd);
            retinfo->status = FAILED;
            return retinfo;
        }
        map_type = (fault_type == COW_FAULT) ? MAP_PRIVATE : MAP_SHARED;

        /* Map the file, if the required fault type is COW_FAULT map the file */
        /* private, else map the file shared. if READ_FAULT is required to be */
        /* generated map the file with read protection else map with read -   */
        /* write protection.                               */

        if ((map_addr = (caddr_t)mmap(0, pagesize*pages_num,
                        ((fault_type == READ_FAULT) ?
			PROT_READ : PROT_READ|PROT_WRITE),
                        map_type, fd, 0))
		      == MAP_FAILED)
        {
            perror("map_and_thread(): mmap()");
            free(empty_buf);
            fflush(NULL);
            remove_files(tmpfile, NULL);
            close(fd);
            retinfo->status = FAILED;
            return retinfo;
        }
        else
        {
            retinfo->mapaddr = map_addr;
            if (verbose_print)
                tst_resm(TINFO,
                  "map_and_thread(): mmap success, address = %p", map_addr);
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

       /*************************************************************/
       /*   The way it was, args could be overwritten by subsequent uses
        *   of it before the called routine had a chance to fully initialize.
        *   This flag stops the overwrite until that routine gets to
        *   begin.  At that point, it is done initializing and it is
        *   safe for the this thread to continue (which will change
        *   args).
        *   A basic race condition.
        */
        thread_begin = TRUE;
        if (pthread_create(&pthread_ids[thrd_ndx++], NULL, exec_func,
                           (void *)&th_args))
        {
            perror("map_and_thread(): pthread_create()");
            thread_begin = FALSE;
            free(empty_buf);
            fflush(NULL);
            remove_files(tmpfile, map_addr);
            close(fd);
            retinfo->status = FAILED;
            return retinfo;
        } else {
            /***************************************************/
            /*   Yield until new thread is done with args.
             */
            while (thread_begin)
                sched_yield();
        }
    } while (thrd_ndx < num_thread);

    if (verbose_print)
        tst_resm(TINFO, "map_and_thread(): pthread_create() success");
    wait_thread = FALSE;
    th_status = malloc(sizeof(int *));

    /* suspend the execution of the calling thread till the execution of the  */
    /* other thread has been terminated.                                      */

    for (thrd_ndx = 0; thrd_ndx < NUMTHREAD; thrd_ndx++)
    {
        if (pthread_join(pthread_ids[thrd_ndx], (void **)th_status))
        {
            perror("map_and_thread(): pthread_join()");
            free(empty_buf);
            fflush(NULL);
            remove_files(tmpfile, map_addr);
            close(fd);
            retinfo->status = FAILED;
            return retinfo;
        }
        else
        {
            if ((int)*th_status == 1)
            {
                tst_resm(TINFO,
                        "thread [%ld] - process exited with errors",
                (long)pthread_ids[thrd_ndx]);
                free(empty_buf);
                remove_files(tmpfile, map_addr);
                close(fd);
                exit(1);
            }
        }
    }

    /* remove the temporary file that was created. - clean up                 */
    /* but dont try to remove special files.                                  */

    /***********************************************/
    /*   Was if !(remove_files()) ...
     *   If that routine succeeds, it returns SUCCESS, which
     *   happens to be 0.  So if the routine succeeded, the
     *   above condition would indicate failure.  This change
     *   fixes that.
     */
    if (remove_files(tmpfile, map_addr) == FAILED)
    {
        free(empty_buf);
        free(th_status);
        retinfo->status = FAILED;
        return retinfo;
    }

    free(empty_buf);
    free(th_status);
    close(fd);
    retinfo->status = SUCCESS;
    return retinfo;
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous read  */
/*              faults in the same address space.                             */
/*                                                                            */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause read faults by simultaneously reading*/
/*              from this memory space.                                       */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Return:      int - status from map_and_thread()                            */
/*                                                                            */
/* Synopsis:    int test1();                                                  */
/*                                                                            */
/******************************************************************************/
static int
test1()
{
    RETINFO_t retval;    /* contains info relating to test status          */

    tst_resm(TINFO, "test1: Test case tests the race condition between "
           "simultaneous read faults in the same address space.");
    map_and_thread("./tmp.file.1", thread_fault, READ_FAULT, NUMTHREAD,
                   &retval);
    return (retval.status);
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous write */
/*              faults in the same address space.                             */
/*                                                                            */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause write faults by simultaneously       */
/*              writing to this memory space.                                 */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Return:      int - status from map_and_thread()                            */
/*                                                                            */
/* Synopsis:    int test2();                                                  */
/*                                                                            */
/******************************************************************************/
static int
test2()
{
    RETINFO_t retval;    /* contains test stats information               */

    tst_resm(TINFO, "test2: Test case tests the race condition between "
       "simultaneous write faults in the same address space.");
    map_and_thread("./tmp.file.2", thread_fault, WRITE_FAULT, NUMTHREAD,
                   &retval);
    return (retval.status);
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous COW   */
/*              faults in the same address space.                             */
/*                                                                            */
/* Description: map a file into memory, create threads and execute a thread   */
/*              function that will cause COW faults by simultaneously         */
/*              writing to this memory space.                                 */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Return:      int - status from map_and_thread()                            */
/*                                                                            */
/* Synopsis:    int test3();                                                  */
/*                                                                            */
/******************************************************************************/
static int
test3()
{
    RETINFO_t retval;    /* contains test stats information                   */

    tst_resm(TINFO, "test3: Test case tests the race condition between "
           "simultaneous COW faults in the same address space.");
    map_and_thread("./tmp.file.3", thread_fault, COW_FAULT, NUMTHREAD, &retval);
    return (retval.status);
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous READ  */
/*              faults in the same address space. File mapped is /dev/zero    */
/*                                                                            */
/* Description: Map a file into memory, create threads and execute a thread   */
/*              function that will cause READ faults by simultaneously        */
/*              writing to this memory space.                                 */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Return:      int - status from map_and_thread()                            */
/*                                                                            */
/* Synopsis:    int test4();                                                  */
/*                                                                            */
/******************************************************************************/
static int
test4()
{
    RETINFO_t retval;     /* contains test status information                 */

    tst_resm(TINFO, "test4: Test case tests the race condition between "
           "simultaneous READ faults in the same address space. "
           "The file mapped is /dev/zero");
    map_and_thread("/dev/zero", thread_fault, COW_FAULT, NUMTHREAD, &retval);
    return (retval.status);
}

/******************************************************************************/
/*                                                                            */
/* Test:    Test case tests the race condition between simultaneous           */
/*         fork - exit faults in the same address space.                      */
/*                                                                            */
/* Description: Initialize large data in the parent process, fork a child and */
/*              and the parent waits for the child to complete execution.     */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Return:      int - status from map_and_thread()                            */
/*                                                                            */
/* Synopsis:    int test5();                                                  */
/*                                                                            */
/******************************************************************************/
static int
test5()
{
    int    fork_ndx = 0;    /* index to the number of processes forked        */
    pid_t  pid      = 0;    /* process id, returned by fork system call.      */
    int    wait_status = 0; /* if status is not NULL store status information */

    tst_resm(TINFO, "test5: Test case tests the race condition between "
           "simultaneous fork - exit faults in the same address space.");

    /* increment the  program's  data  space  by 200*1024 (BRKSZ) bytes       */

    if (sbrk(BRKSZ) == (caddr_t)-1)
    {
        perror("test5(): sbrk()");
        fflush(NULL);
        return FAILED;
    }

    /* fork NUMTHREAD number of processes, assumption is on SMP each will get */
    /* a separate CPU if NRCPUS = NUMTHREAD. The child does nothing; exits    */
    /* immediately, parent waits for child to complete execution.             */
    do
    {
        if (!(pid = fork()))
            _exit (0);
        else
        {
            if (pid != -1)
                wait(&wait_status);
        }

    } while (fork_ndx++ < NUMTHREAD);

    if (sbrk(-BRKSZ) == (caddr_t)-1)
    {
        tst_resm(TINFO, "test5(): rollback sbrk failed");
        fflush(NULL);
        perror("test5(): sbrk()");
        fflush(NULL);
        return FAILED;
    }
    return SUCCESS;
}

/******************************************************************************/
/*                                                                            */
/* Test:        Test case tests the race condition between simultaneous       */
/*              fork - exec - exit faults in the same address space.          */
/*                                                                            */
/* Description: Initialize large data in the parent process, fork a child and */
/*              and the parent waits for the child to complete execution. The */
/*              child program execs a dummy program.                          */
/*                                                                            */
/* Input:       NONE                                                          */
/*                                                                            */
/* Return:      int - status from map_and_thread()                            */
/*                                                                            */
/* Synopsis:    int test6();                                                  */
/*                                                                            */
/******************************************************************************/
static int
test6()
{
    int    res = SUCCESS;
    int    fork_ndx = 0;   /* index to the number of processes forked         */
    pid_t  pid = 0;        /* process id, returned by fork system call.       */
    int    wait_status;    /* if status is not NULL store status information  */
    char  *argv_init[2] =  /* parameter required for dummy function to execvp */
                          {"arg1", NULL};

    tst_resm(TINFO, "test6: Test case tests the race condition between "
           "simultaneous fork -exec - exit faults in the same address space.");

    /* increment the  program's  data  space  by 200*1024 (BRKSZ) bytes       */

    if (sbrk(BRKSZ) == (caddr_t)-1)
    {
        perror("test6(): sbrk()");
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
            if (execvp("dummy", argv_init) == -1)
            {
                if (execvp("./dummy", argv_init) == -1)
		{
		   perror("test6(): execvp()");
                   fflush(NULL);
                   /*************************************************/
                   /*   Dummy uses exit 0 so use something else for
                    *   error exit.
                    */
                    exit(99);
		}
            }
        }
        else
        {
            if (pid != -1)
                wait(&wait_status);
            /*************************************************/
            /*   Dummy uses exit 0.
             *   Capture exit of child and set res accordingly.
             *   It defaults to SUCCESS.  Only gets set if
             *   child fails.
             */
            if (WEXITSTATUS(wait_status) != 0)
               res = FAILED;
        }

    } while (fork_ndx++ < NUMTHREAD);

    if (sbrk(-BRKSZ) == (caddr_t)-1)
    {
        tst_resm(TINFO, "test6(): rollback sbrk failed");
        fflush(NULL);
        perror("test6(): sbrk()");
        fflush(NULL);
        return FAILED;
    }
    /*************************************/
    /*  This used to return SUCCESS, whether or not the
     *  test actually worked.
     */
    return res;
}

/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Desciption:  This is the main function of mmstress program. It processes   */
/*              all command line options and flags and runs the tests.        */
/*              If no specific tests are chosen, all tests will be run one    */
/*              time by default.                                              */
/*                                                                            */
/* Input:       argc - number of command line parameters                      */
/*              argv - pointer to the array of the command line parameters.   */
/*                                                                            */
/* Return:      EXIT_FAILURE - in case sigaction() fails                      */
/*              rc - return code from the testcase                            */
/*                   zero     - success                                       */
/*                   non zero - failure                                       */
/*                                                                            */
/******************************************************************************/
int
main(int   argc,     /* number of command line parameters                     */
     char  **argv)   /* pointer to the array of the command line parameters.  */
{
    extern char *optarg;  /* getopt() function global variables               */
    extern int   optind;  /* index into argument                              */
    extern int   opterr;  /* optarg error detection                           */
    extern int   optopt;  /* stores bad option passed to the program          */

    static char *version_info = "mmstress V1.00 04/17/2001";
                          /* version of this program                          */
    int    (*(test_ptr)[])() =
                               {NULL, test1, test2, test3, test4, test5, test6};
                                    /* pointer to the array of test names     */
    int    ch;                      /* command line flag character            */
    int    test_num     = 0;        /* test number that is to be run          */
    int    test_time    = 0;        /* duration for which test is to be run   */
    int    sig_ndx      = 0;        /* index into signal handler structure.   */
    int    run_once     = TRUE;     /* test/tests are run once by default.    */
    char   *prog_name   = argv[0];  /* name of this program                   */
    int    rc           = 0;        /* return value from tests.  0 - success  */
    int    global_rc    = 0;        /* return value from tests.  0 - success  */
    int    version_flag = FALSE;    /* printf the program version             */
    struct sigaction sigptr;        /* set up signal, for interval timer      */

    static struct signal_info
    {
        int  signum;    /* signal number that hasto be handled                */
        char *signame;  /* name of the signal to be handled.                  */
    }
    sig_info[] =  {     {SIGHUP,"SIGHUP"},
                        {SIGINT,"SIGINT"},
                        {SIGQUIT,"SIGQUIT"},
                        {SIGABRT,"SIGABRT"},
                        {SIGBUS,"SIGBUS"},
                        {SIGSEGV,"SIGSEGV"},
                        {SIGALRM, "SIGALRM"},
                        {SIGUSR1,"SIGUSR1"},
                        {SIGUSR2,"SIGUSR2"},
                        {SIGENDSIG,"ENDSIG"}
                   };

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    optarg = NULL;
    opterr = 0;

    if (argc < 2)
    tst_resm(TINFO, "run %s -h for all options", argv[0]);

    while ((ch = getopt(argc, argv, "hn:p:t:vV")) != -1)
    {
        switch(ch)
        {
            case 'h': usage(argv[0]);
                      break;
            case 'n': if (optarg)
                          test_num = atoi(optarg);
                      else
                          OPT_MISSING(prog_name, optopt);
                      break;
            case 'p': if (optarg)
                          pages_num = atoi(optarg);
                      else
                          OPT_MISSING(prog_name, optopt);
                      break;
            case 't': if (optarg)
                      {
                          tst_resm(TINFO, "Test is scheduled to run for %d hours",
				test_time = atoi(optarg));
                          run_once = FALSE;
                      }
                      else
                          OPT_MISSING(prog_name, optopt);
                      break;
            case 'v': verbose_print = TRUE;
                      break;
            case 'V': if (!version_flag)
                      {
                          version_flag = TRUE;
                          tst_resm(TINFO, "%s: %s", prog_name, version_info);
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

    /* duration for which the tests have to be run. test_time is converted to */
    /* corresponding seconds and the tests are run as long as the current time*/
    /* is less than the required time and test are successul (ie rc = 0)      */

    set_timer(test_time);

    /* set up signals */
    sigptr.sa_handler = (void (*)(int signal))sig_handler;
    sigfillset(&sigptr.sa_mask);
    sigptr.sa_flags = 0;
    for (sig_ndx = 0; sig_info[sig_ndx].signum != -1; sig_ndx++)
    {
        sigaddset(&sigptr.sa_mask, sig_info[sig_ndx].signum);

        if (sigaction(sig_info[sig_ndx].signum, &sigptr,
                      (struct sigaction *)NULL) == SIGENDSIG)
        {
            perror( "main(): sigaction()" );
            fprintf(stderr, "could not set handler for SIGALRM, errno = %d\n",
                    errno);
            exit(EXIT_FAILURE);
        }
    }

    /*************************************************/
    /*   The way this loop was, 5 of 6 tests could fail,
     *   and as long test test 6 passed, the test passed.
     *   The changes in this loop repair that problem.
     */
    do
    {
        if (!test_num)
        {
            int test_ndx;  /* index into the array of tests               */

            for (test_ndx = 1; test_ndx <= MAXTEST; test_ndx++) {
                rc = test_ptr[test_ndx]();
                if (rc == SUCCESS) {
                   tst_resm(TPASS, "TEST %d Passed", test_ndx);
                } else {
                   tst_resm(TFAIL, "TEST %d Failed", test_ndx);
                   global_rc = rc;
                }
            }
        }
        else
        {
            global_rc = (test_num > MAXTEST) ?
		  fprintf(stderr,
                            "Invalid test number, must be in range [1 - %d]\n",
                             MAXTEST):
                  test_ptr[test_num]();
        }

        if (global_rc != SUCCESS) {
            tst_resm(TFAIL, "Test Failed");
            exit(global_rc);
        }

    } while ((TRUE) && (!run_once));

    if (global_rc != SUCCESS) {
        tst_resm(TFAIL, "Test Failed");
    } else {
        tst_resm(TPASS, "Test Passed");
    }
    exit(global_rc);
}