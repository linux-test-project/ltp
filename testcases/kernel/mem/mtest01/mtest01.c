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
 *    Modified.
 *    - Removed compile warnings.
 *    - Added header file #include <unistd.h> definition for getopt()
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  char* mem;
  unsigned int bytecount, maxpercent=0, dowrite=0, verbose=0, j, c;
  unsigned long int maxbytes;
  extern char* optarg;
  size_t chunksize = 1024*1024; /* one meg at a time by default */
  struct sysinfo sstats;

  while((c=getopt(argc, argv, "c:b:p:wvh")) != EOF) {
    switch((char)c) {
      case 'c':
        chunksize = atoi(optarg);
        break;
      case 'b':
        maxbytes = atoi(optarg);
        break;
      case 'p':
        maxpercent = atoi(optarg);
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

  if(maxpercent) {
    unsigned long int D, C;
    sysinfo(&sstats);
    maxbytes = ((float)maxpercent/100)*(sstats.totalram+sstats.totalswap) - ((sstats.totalram+sstats.totalswap)-(sstats.freeram+sstats.freeswap));
    /* Total memory needed to reach maxpercent */
    D = ((float)maxpercent/100)*(sstats.totalram+sstats.totalswap);

    /* Total memory already used */
    C = (sstats.totalram+sstats.totalswap)-(sstats.freeram+sstats.freeswap);

    /* Are we already using more than maxpercent? */
    if(C>D) {
      printf("More memory than the maximum amount you specified is already being used\n");
      exit(1);
    }

    /* set maxbytes to the extra amount we want to allocate */
    maxbytes = D-C;
    printf("Filling up %d%%  of ram which is %lud bytes\n",maxpercent,maxbytes);
  }

  bytecount=chunksize;
  while(1) {
    if((mem = (char*)malloc(chunksize)) == NULL) {
      printf("stopped at %d bytes\n",bytecount);
      exit(1);
    }
    if(dowrite)
      for(j=0; j<chunksize; j++)
        *(mem+j)='a';
    if(verbose) printf("allocated %d bytes chunksize is %d\n",bytecount,chunksize);
    bytecount+=chunksize;
    if(maxbytes && (bytecount >= maxbytes))
      break;
  }
  printf("PASS ... %d bytes allocated.\n",bytecount);
  exit(0);
}
