/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 *
 * The mmap( ) function shall fail if:
 * ML [EAGAIN] The mapping could not be locked in memory,
 * if required by mlockall ( ), due toa lack of resources.
 *
 * Test Step:
 * 1. Call mlockall(), setting MCL_FUTURE;
 * 2. Call setrlimit(), set rlim_cur of resource RLIMIT_MEMLOCK to a
 *    certain value.
 * 3. Map a shared memory object, with size larger than the 
 *    rlim_cur value set in step 2
 * 4. Should get EAGAIN. 
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"
 
#define TNAME "mmap/18-1.c"

int main()
{
  char tmpfname[256];
  int shm_fd;
  
  /* size of shared memory object */
  size_t shm_size;

  void *pa = NULL; 
  void *addr = NULL;
  size_t len;
  int prot = PROT_READ | PROT_WRITE;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;
  
  size_t memlock_size;
  struct rlimit rlim = {.rlim_max = RLIM_INFINITY};
  
  /* Lock all memory page to be mapped */
  if (mlockall(MCL_FUTURE) == -1)
  {
    printf(TNAME " Error at mlockall(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
  }
   
  /* Set rlim.rlim_cur < len */

  len = 1024 * 1024;
  memlock_size = len / 2;
  rlim.rlim_cur = memlock_size;

  /* We don't cate the size of the actual shared memory object */
  shm_size = 1024; 

  if (setrlimit (RLIMIT_MEMLOCK, &rlim) == -1)
  {
		printf(TNAME " Error at setrlimit(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
  }
  
  snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_18_1_%d",
           getpid());
	
  /* Create shared object */
	shm_unlink(tmpfname);
	shm_fd = shm_open(tmpfname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
	if(shm_fd == -1)
	{
		printf(TNAME " Error at shm_open(): %s\n", strerror(errno));
		return PTS_UNRESOLVED;
	}
  shm_unlink(tmpfname);   
  if(ftruncate(shm_fd, shm_size) == -1) {
    printf(TNAME " Error at ftruncate(): %s\n", strerror(errno));
    return PTS_UNRESOLVED;
  }

  fd = shm_fd;	
  pa = mmap (addr, len, prot, flag, fd, off);
  if (pa == MAP_FAILED && errno == EAGAIN)
  {
    printf ("Test Pass: " TNAME " Get EAGAIN: %s\n", 
            strerror(errno));    
    close(fd);
    munmap(pa, len);
    exit(PTS_PASS);
  }
  
  if (pa == MAP_FAILED)
    perror("Error at mmap()");
  close(fd);
  munmap(pa, len);
  printf ("Test Fail: Did not get EAGAIN as expected\n");
  return PTS_FAIL;
}
