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
 * MPR References within the address range starting at pa and 
 * continuing for len bytes to whole pages following the end 
 * of an object shall result in delivery of a SIGBUS signal.
 * 
 * Test step:
 * 1. If Memory Protection option is supported, read the address 
 *    beyond the object (mapped file) size, should get SIGBUS;
 * 2. If Memory Protection option is not supported and if the 
 *    reference does not trigger SIGBUS, make sure the partial
 *    page is zero-filled;
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
 
#define TNAME "mmap/11-2.c"

static int volatile sigbus_flag;

void sigbus_handler (int signum)
{
  printf("Got SIGBUS\n");
  sigbus_flag = 1;
}

int main()
{
  char tmpfname[256];
  long  page_size;
  long total_size; 

  void *pa = NULL; 
  void *addr = NULL;
  size_t len;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  char *ch = NULL;
  struct sigaction sa;
  
  page_size = sysconf(_SC_PAGE_SIZE);
  
  /* Size of the file to be mapped */
  total_size = page_size / 2;
  
  /* mmap will create a partial page */
  len = page_size / 2; 
  
  sigbus_flag = -1;
  sigfillset(&sa.sa_mask);
  sa.sa_handler = sigbus_handler;
  sigaction(SIGBUS, &sa, NULL);

  /* Create tmp file */
  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_11_2_%d",
           getpid());
  unlink(tmpfname);
  fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
  if (fd == -1)
  {  
    printf(TNAME " Error at open(): %s\n", 
           strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
  unlink(tmpfname);
  
  if (ftruncate(fd, total_size) == -1)
  {
    printf(TNAME "Error at ftruncate(): %s\n", 
            strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
    
  prot = PROT_READ | PROT_WRITE;
  flag = MAP_SHARED; 
  off = 0; 
  pa = mmap(addr, len, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf("Test FAIL: " TNAME " Error at mmap(): %s\n",
    	   strerror(errno));    
    exit(PTS_FAIL);
  }
  
  ch = pa + len + 1;

  /* This reference should trigger SIGBUS */
  if (*ch != 0)
  {
  	printf("Test Fail: " TNAME " The partial page at the end of an object "
  		   "is not zero-filled\n");
  	exit(PTS_FAIL);
  }
  
#ifdef _POSIX_MEMORY_PROTECTION
  if (sigbus_flag == 1)
  {
  	printf("Test Pass: " TNAME " Trigger SIGBUS\n");
  	exit(PTS_PASS);
  }
  else if (sigbus_flag == -1)
  {
  	printf("Test Fail: " TNAME " Did not trigger SIGBUS, "
           "while Memory Protection is enabled\n");
  	exit(PTS_PASS);
  }
#endif

  *ch = 'b';
  msync(pa, len, MS_SYNC);
  munmap (pa, len);
  
  prot = PROT_READ | PROT_WRITE;
  flag = MAP_SHARED;  
  pa = mmap(addr, len, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf("Test FAIL: " TNAME " Error at 2nd mmap(): %s\n",
    	   strerror(errno));    
    exit(PTS_FAIL);
  }

  ch = pa + len + 1;
  if (*ch == 'b')
  {
  	printf("Test Fail: " TNAME " Modification of the partial page "
  		   "at the end of an object is written out\n");
  	exit(PTS_FAIL);
  }
  
  close (fd);
  munmap (pa, len);
  printf ("Test Pass\n");
  return PTS_PASS;
}
