/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * Implementation performs mapping operations over whole pages.
 * Thus, while the argument len
 * need not meet a size or alignment constraint, 
 * the implementation shall include, in any mapping
 * operation, any partial page specified by the range [pa,pa+len).
 * The system shall always zero-fill any partial page at the end of an object. 
 * Further, the system shall never write out any modified portions of 
 * the last page of an object which are beyond its end.
 * 
 * Test step:
 * 1. Create a process, in this process: 
      a. map a shared memory object with size of 1/2 * page_size, 
 *       set len = 1/2 * page_size
 *    b. Read the partial page beyond the object size.
 *       Make sure the partial page is zero-filled;
 *    c. Modify a byte in the partial page, then un-map the and close the 
 *       file descriptor.
 * 2. Wait for the child proces to exit, then
 *    Map the shared memory object again, 
 *    read the byte from the position modified at step 1-c and check.
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"
 
#define TNAME "mmap/11-5.c"

int main()
{
  char tmpfname[256];
  long  page_size;
  long total_size; 

  void *pa = NULL, *pa_2 = NULL; 
  void *addr = NULL;
  size_t len;
  int flag;
  int fd, fd_2;
  off_t off = 0;
  int prot;
  
  pid_t child;
  char *ch = NULL, *ch_2 = NULL;
  int exit_val;
 
  page_size = sysconf(_SC_PAGE_SIZE);
  
  /* Size of the file to be mapped */
  total_size = page_size / 2;
  
  /* mmap will create a partial page */
  len = page_size / 2; 

  snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_11_5_%d",
             getpid());
  child = fork();
  if (child == 0)
  {
    /* Create shared object */
    shm_unlink(tmpfname);
    fd = shm_open(tmpfname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
    if(fd == -1)
    {
      printf(TNAME " Error at shm_open(): %s\n", strerror(errno));
      return PTS_UNRESOLVED;
    }
    if(ftruncate(fd, total_size) == -1) {
      printf(TNAME " Error at ftruncate(): %s\n", strerror(errno));
      return PTS_UNRESOLVED;
    }   
  
    prot = PROT_READ | PROT_WRITE;
    flag = MAP_SHARED; 
    off = 0; 
    pa = mmap(addr, len, prot, flag, fd, off);
    if (pa == MAP_FAILED)
    {
      printf("Test FAIL: " TNAME " Error at mmap(): %s\n",
      	     strerror(errno));    
      return PTS_FAIL;
    }
    /* Check the patial page is ZERO filled */ 
    ch = pa + len + 1;
    if (*ch != 0)
    {
      printf("Test Fail: " TNAME " The partial page at the end of an object "
              "is not zero-filled\n");
      return PTS_FAIL;
    }

    /* Write the partial page */
    *ch = 'b';
    munmap (pa, len);
    close (fd);
    return PTS_PASS;
  }	
  wait(&exit_val);
  if (!(WIFEXITED(exit_val) && 
	(WEXITSTATUS(exit_val) == PTS_PASS)))
  {
    shm_unlink(tmpfname);
    return PTS_FAIL;
  }

  fd_2 = shm_open(tmpfname, O_RDWR, 0);
  shm_unlink(tmpfname);   
  
  prot = PROT_READ | PROT_WRITE;
  flag = MAP_SHARED; 
  off = 0; 
  pa_2 = mmap(addr, len, prot, flag, fd_2, off);
  if (pa_2 == MAP_FAILED)
  {
    printf("Test FAIL: " TNAME " Error at 2nd mmap(): %s\n",
            strerror(errno));    
    exit(PTS_FAIL);
  }
    
  ch_2 = pa_2 + len + 1;
  if (*ch_2 == 'b')
  {
    	printf("Test Fail: " TNAME " Modification of the partial page "
     	   "at the end of an object is written out\n");
  	exit(PTS_FAIL);
  }
  close (fd_2);
  munmap (pa_2, len);  
  printf("Test Passed\n");
  return PTS_PASS;
}
