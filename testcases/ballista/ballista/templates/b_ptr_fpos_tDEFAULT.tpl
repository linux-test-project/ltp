// b_ptr_fpos_tDEFAULT.tpl : Ballista Datatype Template for file position pointer
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

name fpos_t* b_ptr_fpos_t;

parent b_ptr_long;

includes
[
{
#include <unistd.h>
#include <fcntl.h>
#include "b_ptr_long.h"
}
]

global_defines
[
{
#define TESTDIR "testdir"
#define TESTFILE        "testdir/testfile_fpos_t"
static FILE* filePtr  = NULL;

}
]

dials
[
  enum_dial VALUE : BEGINNING, MIDDLE, END;
]

access
[
{
  int position = 0;
}

BEGINNING
{
  position = 1;
}
MIDDLE
{
  position = 9;
}
END
{
  position = 72;  // should be EOF
}

{
   int fd;
   char buffer[128];

   // Setup log file for template information
   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }

   if (mkdir(TESTDIR,S_IRWXU|S_IRWXG|S_IRWXO)!=0) /* create test directory, u+rwx */
   {
     //if the directory already exists ignore the error
     if (errno != EEXIST)
     {
       fprintf(logFile,"b_ptr_fpos_t datatype not setup properly - mkdir failed. Function not tested\n");
       fclose(logFile);
       exit(99);
     }
   }

   //remove the file, ignore error if already removed
   if (unlink (TESTFILE) != 0 )
   {
     if (errno !=  ENOENT)
     {
       fprintf(logFile,"unlink failed. Function not tested\n");
       fclose(logFile); 
       exit(99);
     }
   }

   if((fd = open (TESTFILE, O_WRONLY | O_CREAT | O_TRUNC, 0644))==-1)
   {
     fprintf(logFile,"b_ptr_fpos_t datatype not setup properly - open failed. Function not tested\n");
     fclose(logFile);
     exit(99);
   }

   write (fd, "I am a test file.\n", 18);
   write (fd, "I am a test file.\n", 18);
   write (fd, "I am a test file.\n", 18);
   write (fd, "I am a test file.\n", 18);

   if (close (fd)!=0)  
   {
     fprintf(logFile,"b_ptr_fpos_t datatype not setup properly - close failed. Function not tested\n");
     fclose(logFile);
     exit(99);
   }

   if ((filePtr = fopen (TESTFILE,"r+")) == NULL)
   {
     fprintf(logFile,"b_ptr_fpos_t datatype not setup properly - fopen failed.  Function not tested\n");
     fclose(logFile);
     exit(99);
   }
   
   fread(buffer, sizeof(char), position, filePtr);
   
   if (fgetpos(filePtr, _theVariable) != 0)
   {
     fclose(filePtr);
     fprintf(logFile,"b_ptr_fpos_t datatype not setup properly - fgetpos failed.  Function not tested\n");
     fclose(logFile);
     exit(99);
   }

   fclose(logFile);
}
]

commit
[
]

cleanup
[
{
   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }

  if (filePtr != NULL)
  {
    fclose(filePtr);
  }
  
  //remove file
  if (unlink(TESTFILE)!=0)
  {  
    //ignore error if already removed.
    if (errno != ENOENT)
    {
      fprintf(logFile,"unlink testfile_fpos_t failed\n");
    }
  }

  fclose(logFile);
}
]
