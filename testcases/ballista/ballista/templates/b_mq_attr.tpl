// b_mq_attr.tpl : Ballista Datatype Template for message queue attributes 
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

name mq_attr* b_mq_attr;

parent b_ptr_void;

// The POSIX functions states that only the member of mq_attr that will be matter is mq_flags 
// Therefore this member is the focus of the testing

includes
[
 {
   #include "b_ptr_void.h"
   #include <mqueue.h>
   #include <fcntl.h>
 }
]

global_defines
[
 {
   static mq_attr temp_mq_attr;
   static mqd_t temp_mqd;
#define	QUEUE_NAME	"/tmp/ballista_queue"
 }
]

dials
[
  enum_dial HVAL : QUEUE,ZERO,NONBLOCK,MAX,MIN;
]

access
[
QUEUE,NONBLOCK
{
  // mode and attr parameters are necessary
  temp_mqd = mq_open(QUEUE_NAME, O_RDWR|O_CREAT);

  FILE* logFile = NULL;
  if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
  {
    exit(99);
  }

  if (mq_getattr(temp_mqd, &temp_mq_attr) !=0)
  {
    fprintf(logFile,"Problems setting up b_mq_attr QUEUE with mq_getattr - function not tested\n");
    fclose(logFile);
    exit(99);
  }
  fclose(logFile);

}

ZERO
{
  temp_mq_attr.mq_flags =0;
  temp_mq_attr.mq_maxmsg =0;
  temp_mq_attr.mq_msgsize =0;
  temp_mq_attr.mq_curmsgs =0;
}

MAX
{
  temp_mq_attr.mq_flags = MAXLONG;
  temp_mq_attr.mq_maxmsg = MAXLONG;
  temp_mq_attr.mq_msgsize = MAXLONG;
  temp_mq_attr.mq_curmsgs = MAXLONG;
}

MIN
{
  temp_mq_attr.mq_flags = -MAXLONG;
  temp_mq_attr.mq_maxmsg = -MAXLONG;
  temp_mq_attr.mq_msgsize = -MAXLONG;
  temp_mq_attr.mq_curmsgs = -MAXLONG;
}

NONBLOCK
{
  temp_mq_attr.mq_flags |= O_NONBLOCK;
}

{
  _theVariable = &temp_mq_attr;
}
]

commit
[
]

cleanup
[
QUEUE, NONBLOCK
{
  mq_close(temp_mqd);
  mq_unlink(QUEUE_NAME);
}  
]
