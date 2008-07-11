// b_mqd.tpl : Ballista Datatype Template for message queue descriptors
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

name mqd_t b_mqd;

parent b_int;

includes
[
 {
   #include "b_int.h"
   #include <mqueue.h>
   #include <fcntl.h>
 }
]

global_defines
[
 {
   static mqd_t temp_mqd;
#define	QUEUE_NAME	"/tmp/ballista_queue"
 }
]

dials
[
  enum_dial HVAL : OPEN_QUEUE, CLOSE_QUEUE, UNLINK_QUEUE;
]

access
[
{
    // mode and attr parameters are necessary
    temp_mqd = mq_open(QUEUE_NAME, O_RDWR|O_CREAT);

    FILE* logFile = NULL;
    if ((logFile = fopen ("/tmp/templateLog.txt","a+")) == NULL)
    {
      exit(99);
    }
    // fprintf (logFile, "message_queue %d \n", temp_mqd);
}

  CLOSE_QUEUE
  {
    if ((mq_close(temp_mqd)) == -1)
    {
       fprintf (logFile, "b_msg CLOSE_QUEUE - error closing queue - function not tested\n");
       fclose(logFile);
       exit(99);
    }
  }
  UNLINK_QUEUE
  {
    if ((mq_unlink(QUEUE_NAME)) == -1)
    {
       fprintf (logFile, "b_msg UNLINK_QUEUE - error unlinking queue - function not tested\n");
       fclose(logFile);
       exit(99);
    }
  }

{
  fclose(logFile);
  _theVariable = temp_mqd;
}
]

commit
[
]

cleanup
[
{
  mq_close(temp_mqd);
  mq_unlink(QUEUE_NAME);
}  
]
