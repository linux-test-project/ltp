// b_fd.tpl : Ballista Datatype Template for file descriptors
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

name int b_fd;

parent b_int;

includes
[
{
#include <fcntl.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <errno.h>
#include "b_int.h"
}
]

global_defines
[
{
#define TESTDIR "testdir"
#define TESTFILE        "testdir/testfile_fp"
}
]

dials
[
  // Note removed X_GRP, RWX_ALL, RWX_UG from PERMISSIONS to make shorter
  enum_dial MODE : READ, WRITE, APPEND, READ_PLUS, WRITE_PLUS, APPEND_PLUS;
  enum_dial PERMISSIONS : R_USER, W_USER, X_USER, RWX_USER, R_GRP, NONE;
  enum_dial EXISTANCE : EXIST, CLOSED, DELETED, NO_EXIST;   
  enum_dial STATE : EMPTY, BEGINNING, EOF, PAST_EOF;   
]

access
[
{
   mode_t permissionMode;
   int oflag;
   int exist_flag = 1;
   int pos;
   int empty_flag = 0;

   struct stat stat_buf_temp;
   int fd;

   // Setup log file for template information

   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
   //fprintf (logFile, "\nNEW TEST \n");

   if (mkdir(TESTDIR,S_IRWXU|S_IRWXG|S_IRWXO)!=0) /* create test directory, u+rwx */   
   {  
     //if the directory already exists ignore the error
     if (errno != EEXIST) 
     {
       fprintf(logFile,"mkdir failed. Function not tested\n");
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
     fprintf(logFile,"b_fd datatype not setup properly - open failed. Function not tested\n");
     fclose(logFile);
     exit(99);
   }
}

   EMPTY
   {
     empty_flag = 1;
   }

   READ, READ_PLUS, APPEND, APPEND_PLUS
   {    
     if (! empty_flag)
     {
       write (fd, "I am a test file.\n", 18);
     }
   }

{  // all values
   if (close (fd)!=0) 
   {
     fprintf(logFile,"b_fd datatype not setup properly - close failed. Function not tested\n");
     fclose(logFile);
     exit(99);	
   }
}
  NO_EXIST
  {
    exist_flag = 0;
    remove(TESTFILE);
  }

  // using the "default" oflag values used in open to correspond with fopen values.
  READ
  {
    oflag = O_RDONLY;   
  }
  WRITE
  {
    if (exist_flag)
    {
      oflag = O_WRONLY | O_CREAT | O_TRUNC;
    }
    else    
    {
      oflag = O_WRONLY | O_TRUNC;
    }
  }
  APPEND
  {
    if (exist_flag)
    {
      oflag = O_WRONLY | O_CREAT | O_APPEND;
    }
    else     
    {
      oflag = O_WRONLY | O_APPEND;
    }   
  }
  READ_PLUS
  {
    oflag = O_RDWR;    
  }
  WRITE_PLUS
  {
    if (exist_flag)
    {
      oflag = O_RDWR | O_CREAT | O_TRUNC;
    }   
    else        
    {   
      oflag = O_RDWR | O_TRUNC;
    }
  }
  APPEND_PLUS
  {
    if (exist_flag)
    { 
      oflag = O_RDWR | O_CREAT | O_APPEND;
    }   
    else        
    {
      oflag = O_RDWR | O_APPEND;
    }      
  }

  R_USER
  {
    permissionMode = S_IRUSR;
  } 
  W_USER
  {
    permissionMode = S_IWUSR;
  }
  X_USER
  {
    permissionMode = S_IXUSR;
  }
  RWX_USER
  {
    permissionMode = S_IRWXU;
  }
  R_GRP
  {
    permissionMode = S_IRGRP;
  }
  NONE
  {
    permissionMode = 0;
  }

//  X_GRP 
//  {
//    permissionMode = S_IXGRP;
//  }
//  RWX_ALL
//  {
//    permissionMode = S_IRWXU | S_IRWXG | S_IRWXO;
//  } 
//  RWX_UG
//  {
//    permissionMode = S_IRWXU | S_IRWXG;
//  }


{
  if ((fd  = open(TESTFILE, oflag, permissionMode)) == -1)
  {
    if (exist_flag)
    {
      fprintf(logFile,"b_fd datatype not setup properly - open failed\n");
      fclose(logFile);
      exit (99);
    }
  }
}   

  WRITE, WRITE_PLUS
  {
    if (! empty_flag)
    {
      write (fd, "I am a test file.\n", 18);
    }
  }


  BEGINNING
  {
    if (exist_flag && ((pos = lseek(fd, 0L, SEEK_SET)) == -1L))
    {
      fprintf(logFile,"b_fd datatype not setup properly - lseek SET failed\n");
      fclose(logFile);
      exit (99);     
    }
  }
  EOF
  {
    if (exist_flag && ((pos = lseek(fd, 0L, SEEK_END)) == -1L))
    {
      fprintf(logFile,"b_fd datatype not setup properly - lseek END failed\n");
      fclose(logFile);
      exit (99);
    }
  }
  PAST_EOF
  {
    if (exist_flag && ((pos = lseek(fd, 10L, SEEK_END)) == -1L))
    {
      fprintf(logFile,"b_fd datatype not setup properly - lseek END failed\n");
      fclose(logFile);
      exit (99);
    }
  }

{  // generic statement for all values
  _theVariable = fd;
  fclose (logFile);
}
]

commit
[
{
   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
   
   int exist_flag = 1;
}

  NO_EXIST
  {
    exist_flag = 0;
  }

  CLOSED
  {
    if (exist_flag)
    {
      close(_theVariable);
    }
  }

  DELETED
  {
    if (exist_flag)
    {
      // try deleting without closing the file first
      char buffer[128];
      if ((sprintf(buffer, "rm %s", TESTFILE)) <= 0)
      {
        fprintf(logFile,"b_fd datatype not setup properly - rm of the TESTFILE failed. Function not tested\n");
        fclose(logFile);
        exit(99);
      }
      system(buffer);
      //system("ls testdir/testfil* >> testdir/deleteDebug.txt");
    }
  }

{
  fclose(logFile);
}  
]

cleanup
[
{  
   FILE* logFile = NULL;

   if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
   {
      exit(99);
   }
} 

  CLOSED, EXIST
  {
    chown(TESTFILE, getuid(), getgid()); //attempt to change ownership
    chmod(TESTFILE,  S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH);//permits others to read/write; ignore errors
    if (close (_theVariable)!=0) 
    {       //ignore error if file already closed
      if (errno != EBADF) perror("close fname_tempfd");
    }
  }

{
  if (unlink(TESTFILE)!=0)
  {
    //ignore error if already removed.
    if (errno != ENOENT) 
    {
      fprintf(logFile,"unlink fname_testfilename\n");
    }
  }

  //also try to rmdir it because it may be a directory. ignore error messages
  rmdir(TESTFILE);
  fclose(logFile);
}
]
