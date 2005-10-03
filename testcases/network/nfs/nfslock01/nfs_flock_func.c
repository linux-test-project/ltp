#include <stdlib.h>
#include <unistd.h>
                                                                                                                        
#include "nfs_flock.h"

int
lock_reg(fd, type, offset, whence, len, cmd)
   int fd, type, offset;
   int whence, len, cmd;
{
   struct flock lock;

   lock.l_type = type;
   lock.l_start = offset;
   lock.l_whence = whence;
   lock.l_len = len;

   return(fcntl(fd, cmd, &lock));
}

int 
lock_test(fd, type, offset, whence, len)
   int fd, type, offset;
   int whence, len;
{
   struct flock lock;

   lock.l_type = type;
   lock.l_start = offset;
   lock.l_whence = whence;
   lock.l_len = len;

   if (fcntl(fd, F_GETLK, &lock) < 0) {
      perror("F_GETLK");
      exit(2);
   }

   if (lock.l_type == F_UNLCK)
      return(0);

   return(lock.l_pid);
}
