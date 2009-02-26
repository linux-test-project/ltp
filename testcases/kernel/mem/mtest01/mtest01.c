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

/*
 *  FILE        : mtest01.c
 *  DESCRIPTION : mallocs memory <chunksize> at a time until malloc fails.
 *  HISTORY:
 *    04/10/2001 Paul Larson (plars@us.ibm.com)
 *      written
 *    11/09/2001 Manoj Iyer (manjo@austin.ibm.com)
 *      Modified.
 *      - Removed compile warnings.
 *      - Added header file #include <unistd.h> definition for getopt()
 *    05/13/2003 Robbie Williamson (robbiew@us.ibm.com)
 *      Modified.
 *      - Rewrote the test to be able to execute on large memory machines.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"

char *TCID = "mtest01";
int TST_TOTAL = 1;

int pid_count = 0;

void handler(int signo)
{
        pid_count++;
}


int main(int argc, char* argv[]) {
  char* mem;
  float percent;
  unsigned int maxpercent=0, dowrite=0, verbose=0, j, c;
  unsigned long bytecount, alloc_bytes;
  unsigned long long original_maxbytes,maxbytes=0;
  unsigned long long pre_mem, post_mem;
  extern char* optarg;
  int chunksize = 1024*1024; /* one meg at a time by default */
  struct sysinfo sstats;
  int i,pid_cntr;
  pid_t pid,pid_list[1000];
  struct sigaction act;

  act.sa_handler = handler;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction(SIGRTMIN,  &act, 0);

  for (i=0;i<1000;i++)
   pid_list[i]=(pid_t)0;

  while((c=getopt(argc, argv, "c:b:p:wvh")) != EOF) {
    switch((char)c) {
      case 'c':
        chunksize = atoi(optarg);
        break;
      case 'b':
        maxbytes = atoll(optarg);
        break;
      case 'p':
        maxpercent = atoi(optarg);
	if (maxpercent <= 0){
	  tst_resm(TFAIL, "ERROR: -p option requires number greater than 0");
	  exit(1);}
	if (maxpercent > 99){
	  tst_resm(TFAIL, "ERROR: -p option cannot be greater than 99");
	  exit(1);}
        break;
      case 'w':
        dowrite = 1;
        break;
      case 'v':
        verbose = 1;
        break;
      case 'h':
      default:
        printf("Usage: %s [-c <bytes>] [-b <bytes>|-p <percent>] [-v]\n", argv[0]);
        printf("\t-c <num>\tsize of chunk in bytes to malloc on each pass\n");
        printf("\t-b <bytes>\tmaximum number of bytes to allocate before stopping\n");
        printf("\t-p <bytes>\tpercent of total memory used at which the program stops\n");
        printf("\t-w\t\twrite to the memory after allocating\n");
        printf("\t-v\t\tverbose\n");
        printf("\t-h\t\tdisplay usage\n");
        exit(-1);
    }
  }

  sysinfo(&sstats);
  if(maxpercent) {
    unsigned long long total_ram, total_free, D, C;
    percent=(float)maxpercent/100.00;

    total_ram=sstats.totalram;
    total_ram=total_ram+sstats.totalswap;

    total_free=sstats.freeram;
    total_free=total_free+sstats.freeswap;

    /* Total memory used needed to reach maxpercent */
    D = percent*(sstats.mem_unit*total_ram);
    tst_resm(TINFO, "Total memory used needed to reach maxpercent = %llu kbytes", D/1024);

    /* Total memory already used */
    C = sstats.mem_unit*(total_ram-total_free);
    tst_resm(TINFO, "Total memory already used on system = %llu kbytes", C/1024);

    /* Total Free Pre-Test RAM */
    pre_mem = sstats.mem_unit*total_free;

    /* Are we already using more than maxpercent? */
    if(C>D) {
      tst_resm(TFAIL, "More memory than the maximum amount you specified is already being used");
      exit(1);
    }
    else
      pre_mem = sstats.mem_unit*total_free;
   

    /* set maxbytes to the extra amount we want to allocate */
    maxbytes = D-C;
    tst_resm(TINFO, "Filling up %d%% of ram which is %llu kbytes", maxpercent, maxbytes/1024);
  }
  original_maxbytes=maxbytes;
  i=0;
  pid_cntr=0;
  pid=fork();
  if (pid != 0)
    pid_cntr++;
    pid_list[i]=pid;

#if defined (_s390_) /* s390's 31bit addressing requires smaller chunks */
  while( (pid!=0) && (maxbytes > 500*1024*1024) )
  {
    i++;
    maxbytes=maxbytes-(500*1024*1024);
    pid=fork();
    if (pid != 0)
      pid_cntr++;
      pid_list[i]=pid;
  }
  if( maxbytes > 500*1024*1024 )
    alloc_bytes=500*1024*1024;
  else
    alloc_bytes=(unsigned long)maxbytes;

#elif __WORDSIZE==32
  while( (pid!=0) && (maxbytes > 1024*1024*1024) )
  {
    i++;
    maxbytes=maxbytes-(1024*1024*1024);
    pid=fork();
    if (pid != 0)
      pid_cntr++;
      pid_list[i]=pid;
  }
  if( maxbytes > 1024*1024*1024 )
    alloc_bytes=1024*1024*1024;
  else
    alloc_bytes=(unsigned long)maxbytes;

#elif __WORDSIZE==64
  while( (pid!=0) && (maxbytes > (unsigned long long)3*1024*1024*1024) )
  {
    i++;
    maxbytes=maxbytes-(unsigned long long)3*1024*1024*1024;
    pid=fork();
    if (pid != 0)
      pid_cntr++;
      pid_list[i]=pid;
  }
  if( maxbytes > (unsigned long long)3*1024*1024*1024 )
    alloc_bytes=(unsigned long long)3*1024*1024*1024;
  else
    alloc_bytes=(unsigned long)maxbytes;
#endif
 
  if ( pid == 0)			/** CHILD **/
  {
    bytecount=chunksize;
    while(1) {
      if((mem = (char*)malloc(chunksize)) == NULL) {
        tst_resm(TINFO, "stopped at %lu bytes", bytecount);
        exit(1);
      }
      if(dowrite)
        for(j=0; j<chunksize; j++)
          *(mem+j)='a';
      if(verbose)
	tst_resm(TINFO, "allocated %lu bytes chunksize is %d", bytecount, chunksize);
      bytecount+=chunksize;
      if(alloc_bytes && (bytecount >= alloc_bytes))
        break;
    }
    if (dowrite)
      tst_resm(TINFO, "... %lu bytes allocated and used.", bytecount);
    else
      tst_resm(TINFO, "... %lu bytes allocated only.", bytecount);
    kill(getppid(),SIGRTMIN);
    while(1)
      sleep(1);
  }
  else					/** PARENT **/
  {

    i=0;
    sysinfo(&sstats);
   
    if (dowrite)
    {
      /* Total Free Post-Test RAM */
      post_mem = (unsigned long long)sstats.mem_unit*sstats.freeram;
      post_mem = post_mem+((unsigned long long)sstats.mem_unit*sstats.freeswap);
	   
      while ( (((unsigned long long)pre_mem - post_mem) < (unsigned long long)original_maxbytes) &&
              (pid_count < pid_cntr) )
      {
       sleep(1);
       sysinfo(&sstats);
       post_mem = (unsigned long long)sstats.mem_unit*sstats.freeram;
       post_mem = post_mem+((unsigned long long)sstats.mem_unit*sstats.freeswap);
      }
    }
    while (pid_list[i]!=0)
    {
      kill(pid_list[i],SIGKILL);
      i++;
    }
    if (dowrite)
      tst_resm(TPASS, "%llu kbytes allocated and used.", original_maxbytes/1024);
    else
      tst_resm(TPASS, "%llu kbytes allocated only.", original_maxbytes/1024);
  }
  exit(0);
}
