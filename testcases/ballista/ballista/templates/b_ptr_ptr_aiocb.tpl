// b_ptr_ptr_aiocb.tpl : Ballista Datatype Template for a pointer to an aiocb pointer 
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

name structaiocbptrptr b_ptr_ptr_aiocb;

parent b_ptr_void;
// parent restricted as memory runs out 

includes
[
{
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/signal.h>
#include "b_ptr_void.h"
#define structaiocbptrptr struct aiocb**
#define TESTDIR "testdir"
#define TESTFILE        "testdir/testfile_aiocb"
}
]

global_defines
[
{
  static struct aiocb* aiocb_ptr = NULL;
  static struct aiocb aiocb_temp;
  static int fd;
  static char* buf_ptr;
  static char* cp_bufPtr;
}
]

dials
[
  // it would be nice if all the b_fd values could be tested
  enum_dial FD :   
     FD_OPEN_READ,
     FD_OPEN_WRITE,
     FD_OPEN_APPEND_EMPTY,
     FD_OPEN_APPEND_BEGIN,
     FD_OPEN_APPEND_EOF,
     FD_CLOSED,
     FD_DELETED,
     FD_ZERO,
     FD_NEGONE,
     FD_ONE,
     FD_MAXINT,
     FD_MININT,
     FD_READ_ONLY;

  enum_dial OFFSET :
     OFFSET_ZERO,
     OFFSET_ONE,
     OFFSET_NEGONE,
     OFFSET_MAXINT,
     OFFSET_MININT;
//     OFFSET_SIXTYFOUR

  // it would be nice if all the b_ptr_buf values could be tested
  enum_dial BUFFER : 
     BUF_NULL,
     BUF_FILLED_PAGE,
     BUF_ONE_CHAR,
     BUF_CONST;
//     BUF_EMPTY_PAGE,

  enum_dial NBYTES : 
     NBYTE_ZERO,
     NBYTE_ONE,
     NBYTE_MAXULONG;
//     NBYTE_SIXTYFOUR;

  enum_dial REQPRIO :
     PRI_ZERO,
     PRI_ONE,
     PRI_NEGONE,
     PRI_MAXINT,
     PRI_MININT;

  enum_dial SIGEVENT : 
     NONE_SIGSEGV,
     SIGNAL_SIGSEGV,
     ZERO_ZERO,
     MAXINT_MAXINT;

  enum_dial OPCODE :
     OP_ZERO,
     OP_NEGONE,
     OP_MAXINT,
     OP_READ,
     OP_WRITE;
//     OP_NOP;
//     OP_ONE,
//     OP_MININT,

]

access
[
{
   mode_t permissionMode;
   int oflag;

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
     fprintf(logFile,"b_ptr_aiocb datatype not setup properly - open failed. Function not tested\n");
     fclose(logFile);
     exit(99);
   }
}

  FD_OPEN_APPEND_BEGIN, FD_OPEN_APPEND_EOF, FD_READ_ONLY
  {
    write (fd, "I am a test file.\n", 18);
  }


{
   if (close (fd)!=0)
   {
     fprintf(logFile,"b_ptr_aiocb datatype not setup properly - close failed. Function not tested\n");
     fclose(logFile);
     exit(99);
   }
   permissionMode = S_IRWXU | S_IRWXG | S_IRWXO;
}

  FD_OPEN_READ, FD_READ_ONLY
  {
    oflag = O_RDONLY;
  }   
  FD_OPEN_WRITE
  {
    oflag = O_WRONLY | O_CREAT | O_TRUNC;
  }
  FD_OPEN_APPEND_EMPTY, FD_OPEN_APPEND_BEGIN, FD_OPEN_APPEND_EOF
  {
    oflag = O_RDWR | O_CREAT | O_APPEND;
  }
  FD_READ_ONLY
  {
    permissionMode = S_IRUSR;
  }

  FD_OPEN_READ, FD_OPEN_WRITE, FD_OPEN_APPEND_EMPTY, FD_OPEN_APPEND_BEGIN, FD_OPEN_APPEND_EOF, FD_READ_ONLY
  {
    if ((fd  = open(TESTFILE, oflag,  permissionMode)) == -1)
    {
      fprintf(logFile,"b_ptr_aiocb datatype not setup properly - open failed\n");
      fclose(logFile);
      exit (99);
    }
    aiocb_temp.aio_fildes = fd;
  }

  FD_DELETED
  {
    aiocb_temp.aio_fildes = fd;    
    char buffer[128];
    if ((sprintf(buffer, "rm %s", TESTFILE)) <= 0)
    {
      fprintf(logFile,"b_fd datatype not setup properly - rm of the TESTFILE failed. Function not tested\n");
      fclose(logFile);
      exit(99);
    }
    system(buffer);
  }

  FD_OPEN_APPEND_EOF
  {
    int pos;
    if ((pos = lseek(fd, 0L, SEEK_END)) == -1L)
    {
      fprintf(logFile,"b_ptr_aiocb datatype not setup properly - lseek to end of file failed.  Function not tested\n");
      fclose(logFile);
      exit(99);
    }
  }

  FD_CLOSED
  {
    aiocb_temp.aio_fildes = fd;
  }

  FD_ZERO
  {
    aiocb_temp.aio_fildes = 0;
  }
  FD_NEGONE
  {
    aiocb_temp.aio_fildes = -1;
  }
  FD_ONE
  {
    aiocb_temp.aio_fildes = 1;
  }
  FD_MAXINT
  {
    aiocb_temp.aio_fildes = MAXINT;
  }
  FD_MININT
  {
    aiocb_temp.aio_fildes = -MAXINT;
  }

  //----------------------------------------------------------------------
  OFFSET_ZERO
  {
    aiocb_temp.aio_offset = 0;
  }
  OFFSET_ONE
  {
    aiocb_temp.aio_offset = 1;
  }
  OFFSET_NEGONE
  {
    aiocb_temp.aio_offset = -1;
  }
  OFFSET_MAXINT
  {
    aiocb_temp.aio_offset = MAXINT;
  }
  OFFSET_MININT
  {
    aiocb_temp.aio_offset = -MAXINT;
  }
//  OFFSET_SIXTYFOUR
//  {
//    aiocb_temp.aio_offset = 64;
//  }
  //----------------------------------------------------------------------
  BUF_NULL
  {
    aiocb_temp.aio_buf = NULL;
  }
  BUF_FILLED_PAGE
  {
    int pagesize = getpagesize();
    cp_bufPtr = buf_ptr = (char*) malloc(pagesize);
    int i;
    for (i = 0; i < (pagesize-1); i++)
    {
      buf_ptr[i] = 'a';
    }
    buf_ptr[pagesize -1] = '\0';

    aiocb_temp.aio_buf = (void*) buf_ptr;
  }
//  BUF_EMPTY_PAGE
//  {
//    int pagesize = getpagesize();
//    aiocb_temp.aio_buf =  malloc(pagesize);
//  }
  BUF_ONE_CHAR
  {
    cp_bufPtr = buf_ptr = (char*) malloc(sizeof(char));
    aiocb_temp.aio_buf = (void*) buf_ptr;
  }
  BUF_CONST
  {
    aiocb_temp.aio_buf =  (void*)"                                             ";
  }  

  //----------------------------------------------------------------------
  NBYTE_ZERO
  {
    aiocb_temp.aio_nbytes = 0;
  }
  NBYTE_ONE
  {
    aiocb_temp.aio_nbytes = 1;
  }
  NBYTE_MAXULONG
  {
    aiocb_temp.aio_nbytes =  2*MAXLONG + 1;
  }
//  NBYTE_SIXTYFOUR
//  {
//    aiocb_temp.aio_nbytes = 64;
//  }
  
  //----------------------------------------------------------------------
  PRI_ZERO
  {
    aiocb_temp.aio_reqprio = 0;
  }
  PRI_ONE
  {
    aiocb_temp.aio_reqprio = 1;
  }
  PRI_NEGONE
  {
    aiocb_temp.aio_reqprio = -1;
  }
  PRI_MAXINT
  {
    aiocb_temp.aio_reqprio = MAXINT;
  }
  PRI_MININT
  {
    aiocb_temp.aio_reqprio = -MAXINT;
  }
  //----------------------------------------------------------------------
  NONE_SIGSEGV
  {
    aiocb_temp.aio_sigevent.sigev_notify = SIGEV_NONE;
    aiocb_temp.aio_sigevent.sigev_signo = SIGSEGV;
  }
  SIGNAL_SIGSEGV
  {
    aiocb_temp.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    aiocb_temp.aio_sigevent.sigev_signo = SIGSEGV;   
  }
  ZERO_ZERO
  {
    aiocb_temp.aio_sigevent.sigev_notify = 0;
    aiocb_temp.aio_sigevent.sigev_signo = 0;   
  }
  MAXINT_MAXINT
  {
    aiocb_temp.aio_sigevent.sigev_notify = MAXINT;
    aiocb_temp.aio_sigevent.sigev_signo = MAXINT;   
  }

  //----------------------------------------------------------------------
  OP_ZERO
  {
    aiocb_temp.aio_lio_opcode = 0;
  }
//  OP_ONE
//  {
//    aiocb_temp.aio_lio_opcode = 1;
//  }
  OP_NEGONE
  {
    aiocb_temp.aio_lio_opcode = -1;
  }
  OP_MAXINT
  {
    aiocb_temp.aio_lio_opcode = MAXINT;
  }
//  OP_MININT
//  {
//    aiocb_temp.aio_lio_opcode = -MAXINT;
//  }
  OP_READ
  {
    aiocb_temp.aio_lio_opcode = LIO_READ;
  }
  OP_WRITE
  {
    aiocb_temp.aio_lio_opcode = LIO_WRITE;
  }
//  OP_NOP
//  {
//    aiocb_temp.aio_lio_opcode = LIO_NOP;
//  }
{
  fclose(logFile);
  aiocb_ptr = &aiocb_temp;
  _theVariable = &aiocb_ptr;
}
]

commit
[
]

cleanup
[
  FD_OPEN_READ, FD_OPEN_WRITE, FD_OPEN_APPEND_EMPTY, FD_OPEN_APPEND_BEGIN, FD_OPEN_APPEND_EOF, FD_READ_ONLY
  {
    close(fd);
  }

{
  system("rm -f testdir/testfile_aiocb");
  system("rm -rf testdir"); 
}

  // BUF_EMPTY_PAGE
  BUF_FILLED_PAGE, BUF_ONE_CHAR               
  {
    free(cp_bufPtr);
  }

]
