// b_fname.tpl : Ballista Datatype Template for file names
// Copyright (C) 1998-2001  Carnegie Mellon University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

name char* b_fname;

parent b_ptr_char;

includes
[
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "bTypes.h"
#include "b_ptr_char.h"
}
]

global_defines
[
{

#define TESTDIR "testdir/"
#define TESTFILE	"testdir/testfile_fname"
#define TESTSUBDIR "testdir/testsubdir_fname"

#define BMAX_STR 1028
static FILE* filePtr = NULL;
static char *fnameString = NULL;
static char *cp_fnameStr = NULL;
}
]

dials
[
  // Note: removed W_GRP,X_GRP,RWX_UG, RWX_ALL in an effort to speed up tests
  enum_dial PERMISSIONS : R_USER, W_USER, X_USER, RWX_USER, R_GRP, NONE;
  enum_dial STATE : NOT_EXIST_FILE, NOT_EXIST_DIR, OPEN_WRITE, OPEN_READ, CLOSED, LINK;
  enum_dial CONTENTS : EMPTY, NON_EMPTY;
  enum_dial NAME : LOCAL, TMP, EMBEDDED_SPACE, LONG, SPECIAL_CHAR, DIR;
]

access
[
{
  char fname[128];
  char tempStr[64];
  char linkName[BMAX_STR +4];

  int dirFlag = 0;
  int emptyFlag = 0;
  int linkFlag = 0;

  mode_t permissionMode = 777;

  static int fname_count = 0;	/* count for multiple instances	*/
  char fname_count_str [127];	/* count, in string format	*/

  // Setup log file for template information

  FILE* logFile = NULL;

  if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
  {
    exit(99);
  }
  //fprintf (logFile, "\nNEW TEST \n");


   
  if (mkdir(TESTDIR,S_IRWXU|S_IRWXG|S_IRWXO)!=0)	/* create test directory, u+rwx */
  {
    //if the directory already exists ignore the error
    if (errno != EEXIST) 
    {
      fprintf(logFile, "problems creating test directory - b_fname\n");
      fclose(logFile);
      exit(99);
    }
  }
  
  // cleanup existing files - should only be necessary if testing stopped midstream
  system("rm -rf testdir/testfile_fname*");
  system("rm -rf testdir/testsubdir_fname");
  system ("rm -rf /tmp/ballista_test_b_fname*");

}

  R_USER
  {
    permissionMode = 0400;
  }
  W_USER
  {
    permissionMode = 0200;
  } 
  X_USER
  { 
    permissionMode = 0100;
  }
  RWX_USER
  {
    permissionMode = 0700;
  }
  R_GRP
  { 
    permissionMode = 0040;
  }
  NONE
  {
    permissionMode = 0000;
  }

// removed to speed up tests
//  W_GRP
//  {
//    permissionMode = 0020;   
//  }
//  X_GRP
//  {
//    permissionMode = 0010;
//  }
//  RWX_ALL
//  {
//    permissionMode = 0777;
//  }
//  RWX_UG
//  { 
//    permissionMode = 0770;
//  }
 
 
  LOCAL
  {
    fnameString = fname;
    strcpy(fnameString,TESTFILE);
  }
  DIR
  {
    fnameString = fname;
    strcpy(fnameString,TESTSUBDIR);
  }
  EMBEDDED_SPACE
  {
    fnameString = fname;
    strcpy(fnameString,TESTFILE);
    strcat(fnameString,"_Embedded Space");
  }
  LONG
  {
    fnameString = (char*) malloc(sizeof(char) * BMAX_STR);
    if (fnameString == NULL)
    { 
      fprintf (logFile, "b_fname LONG - malloc failed - value not tested\n");
      fclose(logFile);
      exit(99);
    }
    strcpy(fnameString, TESTFILE);
    int i;
    for (i = strlen(fnameString); i < (BMAX_STR -2); i++)
    {
       fnameString[i] = 'a';
    }
    fnameString[BMAX_STR-1] = '\0';
  }

  SPECIAL_CHAR
  {
    fnameString = fname;
    strcpy(fnameString,TESTFILE);
    strcat(fnameString,"_SPECIAL!@#$%^&*(){}[]<>?/-CHAR");
  }

  TMP
  {
    fnameString = fname;
    strcpy(fnameString,"/tmp/ballista_test_b_fname");
  }


  EMPTY
  {
    emptyFlag = 1;
  }

  NON_EMPTY
  {
    emptyFlag = 0;
  }

  NOT_EXIST_FILE
  {
    strcpy(tempStr, "rm -f ");
    strcat(tempStr, fnameString);
    _theVariable = fnameString;
  }

  NOT_EXIST_DIR
  {
    strcpy(tempStr, "rm -rf ");
    strcat(tempStr, TESTDIR);
    system(tempStr);
    _theVariable = fnameString;
  }

  LINK
  {
    linkFlag = 1;
  }

  OPEN_WRITE, OPEN_READ, CLOSED, LINK
  {
    // Create the file with the data  - chmod later for proper permissions
    if (!linkFlag)
    {
      filePtr = fopen(fnameString, "w+");
    }
    else
    {
      strcpy(linkName,fnameString);
      strcat(linkName,"LINK");
      filePtr = fopen(linkName, "w+");
    }      
    if (filePtr == NULL)
    {
      fprintf(logFile, "b_fname OPEN_WRITE, OPEN_READ, LINK, or CLOSED had problems opening file %s - value not tested\n");
      fclose(logFile);
      exit(99);
    } 

    if (!emptyFlag)
    {
      fprintf(filePtr, "This is a test file!\n");
    }
    fclose(filePtr);

    if(!linkFlag)
    {    
      chmod(fnameString, permissionMode);
    }
    else
    {
      chmod(linkName, permissionMode);
    }
  }

  OPEN_READ
  {
    filePtr = fopen(fnameString, "r+");
    if (filePtr == NULL)
    {
      fprintf(logFile, "b_fname OPEN_READ had problems opening file %s - value not tested\n");
      fclose(logFile);
      exit(99);
    }
    _theVariable = fnameString;
  }  

  OPEN_WRITE
  {
    filePtr = fopen(fnameString, "w+");
    if (filePtr == NULL)
    {
      fprintf(logFile, "b_fname OPEN_WRITE had problems opening file - value not tested\n");
      fclose(logFile);
      exit(99);
    }
    _theVariable = fnameString;
  }  

  LINK
  {
    if (link(linkName,fnameString) == -1)
    {
      fprintf(logFile,"b_fname LINK -error creating link - value not tested\n");
      fclose(logFile);
      exit(99);
    }
    chmod(fnameString, permissionMode);
    _theVariable = fnameString;
  } 
{
  cp_fnameStr = fnameString;
  fclose(logFile);
}

]

commit
[
]

cleanup
[
  OPEN_READ, OPEN_WRITE
  {
    fclose(filePtr);
  }

  LINK
  {
    unlink(cp_fnameStr);
  }
  
  LONG
  {
    free(cp_fnameStr);
  }

{
  system ("rm -rf testdir/testfile_fname*");
  system ("rm -rf testdir/testsubdir_fname");
  rmdir(TESTDIR);
}
  TMP
  {
    system ("rm -rf /tmp/ballista_test_b_fname*");
  }
]
