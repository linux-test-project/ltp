// b_ptr_dir.tpl : Ballista Datatype Template for a directory pointer
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

name DIR* b_ptr_dir;

parent b_ptr_char;

includes
[
{
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include "bTypes.h"
#include "b_ptr_char.h"
}
]

global_defines
[
{
#define TESTDIR2	"testdir2"

static DIR* dirPtr;
static DIR* copy_dirPtr;
}
]

dials
[
  enum_dial STATE : DIR_CLOSED,DIR_OPEN,DIR_REMOVED,DIR_MIDWAY;
  enum_dial CONTENT : EMPTY, WITH_FILES;
]

access
[ 
{
  int withFileFlag = 0;
  dirPtr = NULL;
  copy_dirPtr = NULL;

  // cleanup from any prior runs
  system("rm -rf testdir2");

   // Setup log file for template information
   FILE* logFile = NULL;
   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }

  if (mkdir(TESTDIR2,S_IRWXU|S_IRWXG|S_IRWXO)!=0)	/* create test directory, u+rwx */
  {
    //if the directory already exists ignore the error
    if (errno != EEXIST) 
    {
      fprintf (logFile, "b_ptr_dir error creating directory testdir2 - values not tested\n");
      fclose(logFile);
      exit(99);
    }
  }
} 

  WITH_FILES
  {
    char fname[16];
    int i;
    FILE* filePtr = NULL;
    for (i=0; i < 5; i++)
    {
      strcpy(fname,"testfile");
      sprintf (&fname[8], "%d", i);
      if ((filePtr = fopen(fname, "w+")) == NULL)
      {
        fprintf(logFile,"b_ptr_dir WITH_FILES -error creating file - value not tested\n");
        fclose(logFile);
        exit(99);
      }

      fprintf(filePtr, "This is a testfile!\n");
      fclose(filePtr);
    }
    withFileFlag = 1;
  }

{
  if ((dirPtr = opendir(TESTDIR2)) == NULL)
  {
    fprintf (logFile,"b_ptr_dir - error opening directory - value not tested\n");
    fclose(logFile);
    exit(99);
  }
}


  DIR_CLOSED
  {
    if (closedir (dirPtr) == -1) 
    {
      fprintf (logFile, "b_ptr_dir.tpl CLOSED - unable to close directory - value not tested \n");
      fclose(logFile);
      exit(99);
    }
    _theVariable = dirPtr;
    copy_dirPtr = dirPtr; 
  }
  DIR_OPEN
  {	
    _theVariable = dirPtr;     
    copy_dirPtr = dirPtr;
  }
  DIR_MIDWAY
  {
    struct dirent* temp_dirent = NULL;

    if (withFileFlag)
    {
      if ((temp_dirent = readdir(dirPtr))==NULL)
      {
        fprintf(logFile,"b_ptr_dir.tpl DIR_MIDWAY  - error reading through files in directory - value not tested\n");
        fclose(logFile);
        exit(99);
      }
    }
    _theVariable = dirPtr;
    copy_dirPtr = dirPtr;
  }

  DIR_REMOVED
  {
    if (rmdir(TESTDIR2) != 0) 
    {
      fprintf(logFile, "b_ptr_dir DIR_REMOVED - error encountered removing directory - value not tested\n");
      fclose(logFile);
      exit(99);
    }
    _theVariable = dirPtr;
    copy_dirPtr = dirPtr;
  }
{
  fclose(logFile);
}
 
]

commit
[
]

cleanup
[
  DIR_OPEN, DIR_MIDWAY
  {
    closedir(copy_dirPtr);
  }
  
{  
  system("rm -rf testdir2");
}
]
