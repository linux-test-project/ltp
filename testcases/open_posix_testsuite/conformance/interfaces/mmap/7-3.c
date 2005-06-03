/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * MAP_SHARED and MAP_PRIVATE describe the disposition of write references
 * to the memory object. If MAP_SHARED is specified, write references shall
 * change the underlying object. If MAP_PRIVATE is specified, modifications
 * to the mapped data by the calling process shall be visible only to the 
 * calling process and shall not change the underlying object. 
 * It is unspecified whether modifications to the underlying object done 
 * after the MAP_PRIVATE mapping is established are visible through 
 * the MAP_PRIVATE mapping.
 *
 * Test Steps:
 *
 * 1. Create a shared memory object;
 * 2. mmap the shared memory object into memory, setting MAP_PRIVATE;
 * 3. Modify the mapped memory;
 * 4. Fork a child process;
 * 5. Child process mmap the same shared memory object into memory;
 * 6. Check whether the change in step 3 is visible to the child;
 *
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
 
#define TNAME "mmap/7-3.c"

int main()
{
  char tmpfname[256];
  int shm_fd;

  void *pa = NULL; 
  void *addr = NULL;
  size_t size = 1024;
  int prot = PROT_READ | PROT_WRITE;
  int flag;
  int fd;
  off_t off = 0;

  pid_t child;
  char * ch;
  char * ch1;
  
  int exit_stat; 
  
  /* Create shared object */
  snprintf(tmpfname, sizeof(tmpfname), "pts_mmap_7_3_%d",
           getpid());
  shm_unlink(tmpfname);
  shm_fd = shm_open(tmpfname, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
  if(shm_fd == -1)
  {
    printf(TNAME " Error at shm_open(): %s\n", strerror(errno));
	  exit(PTS_UNRESOLVED);
  }
  
  /* Set the size of the shared memory object */
  if(ftruncate(shm_fd, size) == -1) 
  {
    printf(TNAME " Error at ftruncate(): %s\n", strerror(errno));
    exit(PTS_UNRESOLVED);
  }

  fd = shm_fd;
  flag = MAP_PRIVATE;		
  pa = mmap (addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
    printf ("Test Fail: " TNAME " Error at mmap: %s\n", 
            strerror(errno));    
    exit(PTS_FAIL);
  }
  shm_unlink(tmpfname);   
  
  /* Write the mapped memory */

  ch = pa;
  *ch = 'a';
 
  child = fork();
  if (child == 0)
  {
    /* Mmap again the same shared memory to child's memory */
    flag = MAP_PRIVATE;		
    pa = mmap (addr, size, prot, flag, fd, off);
    if (pa == MAP_FAILED)
    {
      printf ("Test Fail: " TNAME " child: Error at mmap: %s\n", 
              strerror(errno));    
      exit(PTS_FAIL);
    }
    
    ch1 = pa;
    
    if (*ch1 == 'a')
    {
	    printf ("Test FAIL: " TNAME "Set flag as MAP_PRIVATE, write reference will "
			        "change the underlying shared memory object\n");
	    exit(PTS_FAIL);
    }
    
    printf ("Test PASS: " TNAME "Set flag as MAP_PRIVATE, write reference will "
  				  "not change the underlying shared memory object\n");
  	exit(PTS_PASS);
	}
  else if (child > 0)
  {
  	waitpid(child, &exit_stat, WUNTRACED);

    close(fd);
    munmap(pa, size);

    if (WIFEXITED(exit_stat))
    	exit(WEXITSTATUS(exit_stat));
    else
    {
    	printf(TNAME " Child does not exit properly\n");
    	exit(PTS_UNRESOLVED);
    }
  }
  else
  {
  	printf (TNAME " Error at fork(), %s\n", strerror(errno));
  	exit(PTS_UNRESOLVED);
  }
}
