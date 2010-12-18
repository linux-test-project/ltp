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
/* along with this program;  if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* File:        mm_core_apis.c                                                */
/*                                                                            */
/* Description: This file tests the libmm core api, the tests are adapted from*/
/*              the tests that were shipped along side the source for libmm   */
/*              written by Ralf S. Engelschall <rse@engelschall.com>          */
/*                                                                            */
/* History:                                                                   */
/* 13 - Jan - 2003 - Created - Manoj Iyer manjo@mail.utexas.edu               */
/* 14 - Jan - 2003 - Added test #1                                            */
/* 16 - Jan - 2003 - Added test #2                                            */
/*                 - TINFO prints testXX instead of mm_core_testXX            */
/*                 - added cleanup() function, currently does nothing.        */
/*                                                                            */
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "test.h"
#include "usctest.h"

#if HAVE_MM_H
#include <mm.h>

#if !defined(TRUE) && !defined(FALSE)
#define TRUE 1
#define FALSE 0
#endif

char *TCID;             /* testcase identifier                                */
int TST_TOTAL = 1;      /* Total number of testcases.                         */

/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Function cleans up temprary file and directories.             */
/*              This function is declared extern since this will be called by */
/*              the LTP-Harness API liberary function tst_brkm().             */
/*                                                                            */
/* Inputs:      NONE                                                          */
/*                                                                            */
/* Return:       exists - tst_exit()                                          */
/*                                                                            */
/******************************************************************************/
extern void
cleanup()
{
    tst_resm(TINFO, "cleanup: cleaning up temporary files and directories\n");
    tst_exit();
}

/******************************************************************************/
/*                                                                            */
/* Function:    mm_core_test01                                                */
/*                                                                            */
/* Description: This testcase will create a shared memory segment using the   */
/*              mm_core_create() low level shared memory API, write certain   */
/*              number of bytes to that memory and try to read it. The shared */
/*              memory size of 16K is created for this test.                  */
/*              Low level APIs used in this test are:                         */
/*              mm_core_create() - creates a shared memory area               */
/*              mm_core_size()   - returns the size in bytes of created mem   */
/*              mm_core_delete() - deletes a shared memory segment            */
/*                                                                            */
/* Inputs:      NONE                                                          */
/*                                                                            */
/* Return:      -1 on error - prints reason using LTP harness APIs            */
/*               0 on success                                                 */
/*                                                                            */
/******************************************************************************/
static int
mm_core_test01()
{
    int           index = 0;        /* index into the allocated mem area      */
    size_t        alloc_size;       /* size of the memory seg allocated       */
    unsigned char *alloc_mem_ptr;   /* pointer to memory alloced by create fn */
    char          *mm_err;          /* error returned by mm_error() function  */

    tst_resm(TINFO, "test01: Testing Memory Segment Access\n");
    tst_resm(TINFO, "test01: Creating 16KB shared memory core area\n");

    if ((alloc_mem_ptr = (unsigned char *)mm_core_create(16*1024, NULL)) == NULL)
    {
        mm_err = (char *)mm_error();
        tst_brkm(TBROK, cleanup, "test01: mm_core_create: %s\n",
                mm_err != NULL ? mm_err : "Unknown error");
        return -1;
    }
    else
    {
        if ((alloc_size = mm_core_size(alloc_mem_ptr)) < 16*1024)
        {
            tst_brkm(TBROK, cleanup, "test01: asked for %d got %d\n",
                    16*1024, alloc_size);
            return -1;
        }
        else
        {
            tst_resm(TINFO, "test01: created shared mem of size: %d\n",
                    alloc_size);
            tst_resm(TINFO,
                    "test01: Writing 0xf5 bytes to memory area\n");
            for (index = 0; index < alloc_size; index++)
                alloc_mem_ptr[index] = 0xf5;

            tst_resm(TINFO,
                    "test01: Reading 0xf5 bytes from memory area\n");
            for (index = 0; index < alloc_size; index++)
            {
                if (alloc_mem_ptr[index] != 0xf5)
                {
                    tst_resm(TFAIL,
                            "At offset %d: alloc_mem_ptr[index] = %#x",
                                index, alloc_mem_ptr[index]);
                    return -1;
                }
            }
            tst_resm(TINFO, "Deleting shared memory segment.\n");
            mm_core_delete(alloc_mem_ptr);
          return 0;
        }
    }
}

/******************************************************************************/
/*                                                                            */
/* Function:      mm_core_test02                                              */
/*                                                                            */
/* Description:                                                               */
/*                Allocate shared memory space, spawn two processes that will */
/*                use this shared memory space. Each process follows the      */
/*                following logic to test locking.                            */
/*                Process A:  - tries to take a lock on the memory            */
/*                    - if not successful                                     */
/*                        - checks if process B has locked it.                */
/*                        - if locked by process B tries to read from this    */
/*                          memory, and if it succeds                         */
/*                          logs a failure.                                   */
/*                    - if lock was a success on this memory segment          */
/*                        - process A writes a certain byte to this memory    */
/*                        - waits for a random period of time (more than 10s) */
/*                        - reads from this memory and verifies the content   */
/*                        - if contents are not the same; flags failure else  */
/*                          flags a success.                                  */
/*                Process A: - unlocks this memory segment.                   */
/*                Low level APIs used in this test are:                       */
/*                mm_core_create() - creates a shared memory area             */
/*                mm_core_size()   - returns the size in bytes of created mem */
/*                mm_core_delete() - deletes a shared memory segment          */
/*                                                                            */
/* Inputs:        NONE                                                        */
/*                                                                            */
/* Return:        -1 on error - prints reason using LTP harness APIs          */
/*                 0 on success                                               */
/*                                                                            */
/******************************************************************************/
static int
mm_core_test02()
{
    int           indexchld;        /* index for child into allocated memory  */
    int           indexprnt;        /* index for parent into allocated memory */
    int           locked_by_child;  /* flag indicates if child locked memory  */
    int           locked_by_parent; /* flag indicates if parent locked memory */
    int           exitstat;         /* exit status of the child process       */
    pid_t         pid;              /* child process id.                      */
    size_t        alloc_size;       /* size of allocated memory segment       */
    unsigned char *alloc_mem_ptr;   /* pointer to memory alloced by create fn */
    char          *mm_err;          /* error returned by mm_error() function  */

    tst_resm(TINFO, "test02: Testing Memory Locking\n");
    tst_resm(TINFO, "test02: Creating shared memory core area\n");

    if ((alloc_mem_ptr = (unsigned char *)mm_core_create(16*1024, NULL)) == NULL)
    {
        mm_err = (char *)mm_error();
        tst_brkm(TBROK, cleanup, "test02: mm_core_create: %s\n",
                mm_err != NULL ? mm_err : "Unknown error");
        return -1;
    }
    else
    {
        if ((alloc_size = mm_core_size(alloc_mem_ptr)) < 16*1024)
        {
            tst_brkm(TBROK, cleanup, "test01: asked for %d got %d\n",
                    16*1024, alloc_size);
            return -1;
        }
        if ((pid = fork()) == 0)
        {
            /* CHILD CODE */
            tst_resm(TINFO,
                "test02: child: locking shared memory\n");
            if (!mm_core_lock(alloc_mem_ptr, MM_LOCK_RW))
            {
                tst_resm(TINFO,
                    "test02: child: failed to take a lock: %s\n",
                        mm_err != NULL ? mm_err : "Unknown error");
                if (!locked_by_parent)
                {
                    tst_resm(TFAIL,
                        "test02: child: mem not locked by parent, "
                        "and child failed to take lock\n");
                    exit(-1);
                }
                else
                {
                    tst_resm(TINFO,
                        "test02: child: read from memory locked by parent\n");
                    for (indexchld = 0; indexchld < alloc_size; indexchld++)
                    {
                        if (alloc_mem_ptr[indexchld] == 0xf5)
                        {
                            tst_resm(TFAIL,
                             "test02: child: can read mem locked by parent\n");
                             exit(-1);
                        }
                    }
                }
            }
            else
            {
                locked_by_child = TRUE;
                tst_resm(TINFO,
                    "test02: child: lock accuired\n");
                tst_resm(TINFO, "test02: child: writing 0xf4 to memory\n");
                for (indexchld = 0; indexchld < alloc_size; indexchld++)
                    alloc_mem_ptr[indexchld] = 0xf5;
                usleep(10);
                tst_resm(TINFO,
                 "test02: child: check if parent could overwrite mem\n");
                for (indexchld = 0; indexchld < alloc_size; indexchld++)
                {
                    if (alloc_mem_ptr[indexchld] != 0xf4)
                    {
                        tst_resm(TFAIL,
                         "test02: child: parent wrote mem locked by child\n");
                        exit(-1);
                    }
                }
                tst_resm(TINFO,
                 "test02: child: parent did not overwite child mem\n");
                tst_resm(TINFO,
                 "test02: child: unlocking memory\n");
                if (!mm_core_unlock(alloc_mem_ptr))
                {
                    tst_brkm(TINFO, cleanup,
                        "test02: child: failed to unlock: %s\n",
                            mm_err != NULL ? mm_err : "Unknown error");
                    exit(-1);
                }
                locked_by_child = FALSE;
                exit(0);
            }
        }
        /* PARENT CODE */
        tst_resm(TINFO,
            "test02: parent: locking shared memory\n");
        if (!mm_core_lock(alloc_mem_ptr, MM_LOCK_RW))
        {
            tst_resm(TINFO,
                "test02: parent: failed to take a lock: %s\n",
                    mm_err != NULL ? mm_err : "Unknown error");
            if (!locked_by_child)
            {
                kill(pid, SIGTERM);
                tst_resm(TFAIL,
                    "test02: parent: !locked by child  parent fail to lock\n");
                tst_resm(TINFO, "test02: Deleting shared memory core area\n");
                mm_core_delete(alloc_mem_ptr);
                return -1;
            }
            else
            {
                tst_resm(TINFO,
                    "test02: parent: read from memory locked by child\n");
                for (indexprnt = 0; indexprnt < alloc_size; indexprnt++)
                {
                    if (alloc_mem_ptr[indexprnt] == 0xf5)
                    {
                        tst_resm(TFAIL,
                         "test02: parent: can read mem locked by child\n");
                        kill(pid, SIGTERM);
                        tst_resm(TINFO,
                                "test02: Deleting shared memory core area\n");
                        mm_core_delete(alloc_mem_ptr);
                        return -1;
                    }
                }
            }
        }
        else
        {
            locked_by_parent = TRUE;
            tst_resm(TINFO,
                "test02: parent: lock accuired\n");
            tst_resm(TINFO, "test02: parent: writing 0xf5 to memory\n");
            for (indexprnt = 0; indexprnt < alloc_size; indexprnt++)
                alloc_mem_ptr[indexprnt] = 0xf5;
            usleep(10);
            tst_resm(TINFO,
             "test02: parent: check if child could overwrite mem\n");
            for (indexprnt = 0; indexprnt < alloc_size; indexprnt++)
            {
                if (alloc_mem_ptr[indexprnt] != 0xf5)
                {
                    tst_resm(TFAIL,
                     "test02: parent: child wrote mem locked by parent\n");
                    kill(pid, SIGTERM);
                    tst_resm(TINFO,
                            "test02: Deleting shared memory core area\n");
                    mm_core_delete(alloc_mem_ptr);
                    return -1;
                }
            }
            tst_resm(TINFO,
             "test02: parent: child did not overwite parent mem\n");
            tst_resm(TINFO,
             "test02: parent: unlocking memory\n");
            if (!mm_core_unlock(alloc_mem_ptr))
            {
                tst_brkm(TINFO, cleanup,
                    "test02: parent: failed to unlock: %s\n",
                        mm_err != NULL ? mm_err : "Unknown error");
                tst_resm(TINFO, "test02: Deleting shared memory core area\n");
                mm_core_delete(alloc_mem_ptr);
                return -1;
            }
            locked_by_parent = FALSE;
            waitpid(pid, &exitstat, 0);
            if (WEXITSTATUS(exitstat))
            {
                tst_resm(TINFO, "test02: parent: child exited with: %d\n",
                        WEXITSTATUS(exitstat));
                return -1;
            }
        }
    }
    tst_resm(TINFO, "test02: Deleting shared memory core area\n");
    mm_core_delete(alloc_mem_ptr);
  return 0;
}

/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Description: Entry point to this program, ...                              */
/*                                                                            */
/******************************************************************************/

int
main(int  argc,            /* argument count                                  */
         char *argv[])     /* argument array                                  */
{
    int ret = 0;           /* return value from the tests                     */

    TCID = "mm_core_apis"; /* identify the testsuite                          */
    tst_resm(TINFO, "Testing Low-Level Shared memory API\n");

    TCID = "mm_core_test01";    /* identify the first testcase                */
    if ((ret = mm_core_test01()) == 0)
    {
        tst_resm(TPASS,
                "mm_core_test01: Testing Memory Segment Access success");
    }

    TCID = "mm_core_test02";    /* identify the first testcase                */
    if ((ret = mm_core_test02()) == 0)
    {
        tst_resm(TPASS,
                "mm_core_test01: Testing Memory Locking success");
    }
    exit(ret);
}
#else
int main(void) {
    tst_resm(TCONF, "System doesn't have libmm support\n");
    tst_exit();
}
#endif