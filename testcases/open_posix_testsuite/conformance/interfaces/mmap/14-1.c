/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * The st_ctime and st_mtime fields of a file that is mapped with
 * MAP_SHARED and PROT_WRITE shall be marked for update at some point
 * in the interval between a write reference to the
 * mapped region and the next call to msync() with MS_ASYNC or MS_SYNC
 * for that portion of the file by any process.
 * If there is no such call and if the underlying file is modified
 * as a result of a write reference, then these fields shall be marked
 * for update at some time after the write reference.
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

#define TNAME "mmap/14-1.c"

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
  struct stat stat_buff;

  total_size = 1024;
  size = total_size;

  time_t mtime1, mtime2, ctime1, ctime2;

  char *ch;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_14_1_%d",
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
  sleep(1);
  printf("Time before write reference: %ld\n", time(NULL));
  /* Before write reference */
  if (stat(tmpfname, &stat_buff) == -1)
  {
    printf(TNAME " Error at 1st stat(): %s\n",
           strerror(errno));
    unlink(tmpfname);
    exit(PTS_UNRESOLVED);
  }

  ctime1 = stat_buff.st_ctime;
  mtime1 = stat_buff.st_mtime;

  ch = pa;
  *ch = 'b';

  /* Wait a while in case the precision of the sa_time
  * is not acurate enough to reflect the update
  */
  sleep(1);
  printf("Time before msync(): %ld\n", time(NULL));
  msync(pa, size, MS_SYNC);

  /* FIXME: Update the in-core meta data to the disk */
  fsync(fd);

  if (stat(tmpfname, &stat_buff) == -1)
  {
    printf(TNAME " Error at stat(): %s\n",
           strerror(errno));
    unlink(tmpfname);
    exit(PTS_UNRESOLVED);
  }

  ctime2 = stat_buff.st_ctime;
  mtime2 = stat_buff.st_mtime;

  printf("ctime1: %ld, ctime2: %ld\nmtime1: %ld, mtime2: %ld\n",
                  ctime1, ctime2, mtime1, mtime2);
  if (ctime2 == ctime1 || mtime2 == mtime1)
  {
    printf("Test Fail " TNAME
           " st_ctime and st_mtime were not updated properly\n");
    unlink(tmpfname);
    exit(PTS_FAIL);
  }

  munmap(pa, size);
  close(fd);
  printf("Test Pass\n");
  return PTS_PASS;
}