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
/* along with this program;  if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    */
/*                                                                              */
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
/*                                                                            */
/******************************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "test.h"
#include "usctest.h"

char *TCID;             /* testcase identifier                                */
int TST_TOTAL = 1;      /* Total number of testcases.                         */
extern int Tst_count;   /* Testcase couter for tst_*** routines.              */


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

    tst_resm(TINFO, "mm_core_test01: Testing Memory Segment Access\n");
    tst_resm(TINFO, "mm_core_test01: Creating 16KB shared memory core area\n");
    
    if ((alloc_mem_ptr = (char *)mm_core_create(16*1024, NULL)) == NULL)
    {
        mm_err = (char *)mm_error();
        tst_resm(TBROK, "mm_core_test01: mm_core_create: %s\n", 
                mm_err != NULL ? mm_err : "Unknown error");
        return -1;
    }
    else
    {
        if ((alloc_size = mm_core_size(alloc_mem_ptr)) < 16*1024)
        {
            tst_resm(TBROK, "mm_core_test01: asked for %d got %d\n", 16*1024,
                    alloc_size);
            return -1;
        }
        else
        {
            tst_resm(TINFO, "mm_core_test01: created shared mem of size: %d\n",
                    alloc_size);
            tst_resm(TINFO, 
                    "mm_core_test01: Writing 0xf5 bytes to memory area\n");
            for (index = 0; index < alloc_size; index++)
                alloc_mem_ptr[index] = 0xf5;

            tst_resm(TINFO, 
                    "mm_core_test01: Reading 0xf5 bytes from memory area\n");
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
    unsigned char *alloc_mem_ptr;   /* pointer to memory alloced by create fn */
    char          *mm_err;          /* error returned by mm_error() function  */

    tst_resm(TINFO, "mm_core_test02: Testing Memory Locking\n");
    tst_resm(TINFO, "mm_core_test02: Creating shared memory core area\n");

    if ((alloc_mem_ptr = (char *)mm_core_create(16*1024, NULL)) == NULL)
    {
        mm_err = (char *)mm_error();
        tst_resm(TBROK, "mm_core_test02: mm_core_create: %s\n", 
                mm_err != NULL ? mm_err : "Unknown error");
        return -1;
    }
    else
    {
	}

}


/******************************************************************************/
/*                                                                            */
/* Function:    main                                                          */
/*                                                                            */
/* Description: Entry point to this program, ...                              */
/*                                                                            */
/******************************************************************************/

int main(int  argc,        /* argument count                                  */
         char *argv[])  /* argument array                                     */
{
    int ret;    /* return value from the tests                                */

    TCID = "mm_core_apis";        /* identify the testsuite                   */
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
    tst_exit();
}
