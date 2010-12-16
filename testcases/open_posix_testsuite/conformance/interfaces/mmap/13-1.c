/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The st_atime field of the mapped file may be marked for update
 * at any time between the mmap() call and the corresponding munmap()
 * call. The initial read or write reference to a mapped region
 * shall cause the file¡¯s st_atime field to be marked for update if
 * it has not already been marked for update.
 *
 * Test Steps:
 * 1. Do stat before mmap() and after munmap(),
 *    also after writing the mapped region.
 * 2. Compare whether st_atime has been updated.
 */

 /* adam.li@intel.com: On linux, it looks mmap() will update
  * st_atime, write to the mapped region will not cause the update
  * */

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

#define TNAME "mmap/13-1.c"

int main()
{
  char tmpfname[256];
  char* data;
  long total_size;

  void *pa = NULL;
  void *addr = NULL;
  size_t size;
  int flag;
  int fd;
  off_t off = 0;
  int prot;

  total_size = 1024;
  size = total_size;

  struct stat stat_buff, stat_buff2;
  time_t atime1, atime2, atime3;

  char *ch;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_13_1_%d",
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

  data = (char *) malloc(total_size);
  memset(data, 'a', total_size);
  printf("Time before write(): %ld\n", time(NULL));
  if (write(fd, data, total_size) != total_size)
  {
    printf(TNAME "Error at write(): %s\n",
            strerror(errno));
    unlink(tmpfname);
    exit(PTS_UNRESOLVED);
  }
  free(data);

  if (stat(tmpfname, &stat_buff) == -1)
  {
    printf(TNAME " Error at 1st stat(): %s\n",
           strerror(errno));
    unlink(tmpfname);
    exit(PTS_UNRESOLVED);
  }
  /* atime1: write */
  atime1 = stat_buff.st_atime;

  sleep(1);

  flag = MAP_SHARED;
  prot = PROT_READ | PROT_WRITE;
  printf("Time before mmap(): %ld\n", time(NULL));
  pa = mmap(addr, size, prot, flag, fd, off);
  if (pa == MAP_FAILED)
  {
  	printf ("Test Fail: " TNAME " Error at mmap: %s\n",
            strerror(errno));
    unlink(tmpfname);
    exit(PTS_FAIL);
  }

  if (stat(tmpfname, &stat_buff2) == -1)
  {
    printf(TNAME " Error at 2nd stat(): %s\n",
           strerror(errno));
    unlink(tmpfname);
    exit(PTS_UNRESOLVED);
  }
  /* for mmap */
  atime2 = stat_buff2.st_atime;

  /* Wait a while in case the precision of the sa_time
  * is not acurate enough to reflect the change
  */
  sleep(1);

  /* write reference to mapped memory */
  ch = pa;
  *ch = 'b';

  printf("Time before munmap(): %ld\n", time(NULL));
  munmap(pa, size);

  /* FIXME: Update the in-core meta data to the disk */
  fsync(fd);
  close(fd);
  if (stat(tmpfname, &stat_buff) == -1)
  {
    printf(TNAME " Error at 3rd stat(): %s\n",
           strerror(errno));
    unlink(tmpfname);
    exit(PTS_UNRESOLVED);
  }
  /* atime3: write to memory */
  atime3 = stat_buff.st_atime;

  printf("atime1: %d, atime2: %d, atime3: %d\n",
         (int)atime1, (int)atime2, (int)atime3);
  if (atime1 != atime3 ||
  	  atime1 != atime2)
  {
  	printf ("Test Pass\n");
    unlink(tmpfname);
  	exit(PTS_PASS);
  }

  printf("Test Fail " TNAME " st_atime did not update properly\n");
  unlink(tmpfname);
  return PTS_FAIL;
}