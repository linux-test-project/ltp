/*
 *
 *   Copyright (c) International Business Machines  Corp., 2002
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

/*
 * Test Name: ftruncate03
 *
 * Test Description:
 *  Verify that,
 *  1) ftruncate(2) returns -1 and sets errno to EINVAL if the specified
 *     socket is invalid.
 *  2) ftruncate(2) returns -1 and sets errno to EINVAL if the specified
 *     file descriptor has an attempt to write, when open for read only.
 *  3) ftruncate(2) returns -1 and sets errno to EBADF if the file descriptor
 *     of the specified file is not valid.
 *
 * Expected Result:
 *  ftruncate() should fail with return value -1 and set expected errno.
 *
 * HISTORY
 *      02/2002 Written by Jay Huie
 *	02/2002 Adapted for and included into the LTP by Robbie Williamson
 *
 * RESTRICTIONS:
 *  This test should be run by 'non-super-user' only.
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/fcntl.h>

#include "test.h"
#include "usctest.h"

#define TESTFILE	"ftruncate03_tst_file"

char *TCID="ftruncate03";
int TST_TOTAL=3;

int main()
{
   int wjh_ret = -1, wjh_f = -1, count = 0;
   //used for the 2nd test
   //make str > trunc_size characters long
   char str[] = "THIS IS JAYS TEST FILE DATA";
   int trunc_size = 4; 
   int flag = O_RDONLY;

   printf("Starting test, possible errnos are; EBADF(%d) EINVAL(%d)\n",
                                               EBADF, EINVAL);
   printf("\t\tENOENT(%d) EACCES(%d) EPERM(%d)\n\n", ENOENT, EACCES, EPERM);

   tst_tmpdir();

//TEST1: ftruncate on a socket is not valid, should fail w/ EINVAL

   printf("Starting test1\n");
   wjh_f = socket(PF_INET, SOCK_STREAM, 0);
   wjh_ret = ftruncate(wjh_f, 1);
#ifdef DEBUG
   printf("DEBUG: fd: %d ret: %d errno(%d) %s\n",
                wjh_f, wjh_ret, errno, strerror(errno));
#endif
   if (wjh_ret == -1 && errno == EINVAL)
   {   printf("Test Failed as Expected!\n"); }
   else 
   {   printf("ftruncate(socket) Failed! Should be-> "
               "ret: -1 errno: EINVAL(%d)\n", EINVAL);
   }
   close(wjh_f); errno = 0; wjh_ret = 0; wjh_f = -1; 

//TEST2: ftruncate on fd not open for writing should be EINVAL

   printf("\nStarting test2\n");
   //create a file and fill it so we can truncate it in ReadOnly mode
   //delete it first, ignore if it doesn't exist
   unlink(TESTFILE); errno = 0;
   wjh_f = open(TESTFILE, O_RDWR|O_CREAT, 0644);
   if (wjh_f == -1) { perror("Can't open file"); exit(-1); }
   while (count < strlen(str) )
   {
      if ((count += write(wjh_f, str, strlen(str))) == -1) 
      { perror("Write failed!\n"); close(wjh_f); exit(-1); } 
   }
   close(wjh_f); errno = 0; 

//Uncomment below if you want it to succeed, O_RDWR => success
// flag = O_RDWR;
   if (flag == O_RDWR) { printf("\tLooks like it should succeed!\n"); }

   wjh_f = open(TESTFILE, flag);
   if (wjh_f == -1) { perror("Can't open file"); exit(-1); }
   wjh_ret = ftruncate(wjh_f, trunc_size);
#ifdef DEBUG
   printf("DEBUG: fd: %d ret: %d @ errno(%d) %s\n",
                wjh_f, wjh_ret, errno, strerror(errno));
#endif
   if ((flag == O_RDONLY) && (wjh_ret == -1) && (errno == EINVAL))
   {   printf("Test failed as Expected!\n"); }
   else if ((flag == O_RDWR))
   {   if (wjh_ret == 0) { printf("Test Succeeded!\n"); }
       else { printf("Truncate should have succeeded, but didn't! ret: "
                      "%d errno(%d) %s\n", wjh_ret, errno, strerror(errno)); }
   }
   else //flag was O_RDONLY but return codes wrong
   {   printf("ftruncate(rd_only_fd) Failed! Should be-> "
               "ret: -1 errno: EINVAL(%d)\n", EINVAL);
   }
   close(wjh_f); errno = 0; wjh_ret = 0; wjh_f = -1; 

//TEST3: invalid socket descriptor should fail w/ EBADF

   printf("\nStarting test3\n");
   wjh_f = -999999; //should be a bad file descriptor
   wjh_ret = ftruncate(wjh_f, trunc_size);
#ifdef DEBUG
   printf("DEBUG: fd: %d ret: %d @ errno(%d) %s\n",
                wjh_f, wjh_ret, errno, strerror(errno));
#endif
   if (wjh_ret != -1 && errno != EBADF)
   {   printf("ftruncate(invalid_fd) failed! Should be-> "
               "ret: -1 errno: EBADF(%d)\n", EBADF);
   }
   else { printf("Test failed as Expected!\n"); }

/* Remove tmp dir and all files in it */
   tst_rmdir();

//Done Testing
   return 0;
}

