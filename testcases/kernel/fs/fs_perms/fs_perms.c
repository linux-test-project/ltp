/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
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
 *  FILE(s)     : fs_perms.c simpletest.sh textx.o Makefile README
 *  DESCRIPTION : Regression test for Linux filesystem permissions.
 *  AUTHOR      : Jeff Martin (martinjn@us.ibm.com)
 *  HISTORY     : 
 *     (04/12/01)v.99  First attempt at using C for fs-regression test.  Only tests read and write bits.
 *     (04/19/01)v1.0  Added test for execute bit.
 *     (05/23/01)v1.1  Added command line parameter to specify test file.
 *     (07/12/01)v1.2  Removed conf file and went to command line parameters.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int testsetup(mode_t mode, int cuserId, int cgroupId);
int testfperm(int userId, int groupId, char* fperm);

int main( int argc, char *argv[]) {
   char fperm[1];
   int result, exresult=0,  cuserId=0, cgroupId=0, userId=0, groupId=0;
   mode_t mode;

   switch (argc) {
      case 8:
	      mode = strtol(argv[1],(char**)NULL,010);
              cuserId = atoi(argv[2]);
              cgroupId = atoi(argv[3]);
              userId = atoi(argv[4]);
              groupId = atoi(argv[5]);
              fperm[0] = *argv[6];
              exresult = atoi(argv[7]);
	      break;
      default:
	      printf("Usage: %s <mode of file> <UID of file> <GID of file> <UID of tester> <GID of tester> <permission to test r|w|x> <expected result as 0|1>\n",argv[0]); 
	      exit(0);
   }

   testsetup(mode,cuserId,cgroupId);
   result=testfperm(userId,groupId,fperm);
   system("rm test.file");
   printf("%c a %03o file owned by (%d/%d) as user/group(%d/%d)  ",fperm[0],mode,cuserId,cgroupId,userId,groupId);
   if (result == exresult) {
      printf("PASS\n");
      exit(0);
      }
   else {
      printf("FAIL\n");
      exit(1);
      }
}

int testsetup(mode_t mode, int cuserId, int cgroupId) {
   system("cp testx.file test.file");
   chmod("test.file",mode);
   chown("test.file",cuserId,cgroupId);
   return(0);
}

int testfperm(int userId, int groupId, char* fperm) {

    FILE *testfile;
    pid_t PID;
    int tmpi,nuthertmpi;

/*  SET CURRENT USER/GROUP PERMISSIONS */
    if(setegid(groupId)) {
        printf("could not setegid to %d.\n",groupId);
           seteuid(0);
           setegid(0);
           return(-1);
        }   
    if(seteuid(userId)) {
        printf("could not seteuid to %d.\n",userId);
           seteuid(0);
           setegid(0);
           return(-1);
        }

    switch(tolower(fperm[0])) {
       case 'x': 
          PID = fork();
	  if (PID == 0) {
             execlp("./test.file","test.file",NULL); 
	     exit(0);
	  }
	  wait(&tmpi);
	  nuthertmpi=WEXITSTATUS(tmpi); 
          seteuid(0);
          setegid(0);
	  return(nuthertmpi);

       default: 
          if((testfile=fopen("test.file",fperm))){
            fclose(testfile);
            seteuid(0);
            setegid(0);
            return (1);
	  }
          else {
            seteuid(0);
            setegid(0);
            return (0);
	  }
    }
}
