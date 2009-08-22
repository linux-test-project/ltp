
/*
 * Copyright (c) 2002, Intel Corporation.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */


#include <sys/select.h>


int sync_pipe_create(int fd[])
{
        return pipe (fd);
}

int sync_pipe_close(int fd[])
{
        int r = 0;

        if (fd[0] != -1)
                r = close (fd[0]);
        if (fd[1] != -1)
                r |= close (fd[1]);
        return r;
}

int sync_pipe_wait(int fd[])
{
        char buf;
        int r;

        if (fd[1] != -1) {
                close (fd[1]);
                fd[1] = -1;
        }

        r = read (fd[0], &buf, 1);

        if ((r != 1) || (buf != 'A'))
                return -1;
        return 0;
}

int sync_pipe_wait_select(int fd[], long tv_sec)
{
        int r;
       fd_set rfds;
       struct timeval tv;
       int err;

       tv.tv_sec = tv_sec;
       tv.tv_usec = 0;

        if (fd[1] != -1) {
                close (fd[1]);
                fd[1] = -1;
        }

       FD_ZERO(&rfds);
       FD_SET(fd[0], &rfds);

       r = select(fd[0] + 1, &rfds, NULL, NULL, &tv);
       err = errno;

       if (FD_ISSET(fd[0], &rfds)) {
               return sync_pipe_wait(fd);
       }

       return r ? err : -ETIMEDOUT;
}


int sync_pipe_notify(int fd[])
{
        char buf = 'A';
        int r;

        if (fd[0] != -1) {
                close (fd[0]);
                fd[0] = -1;
        }

        r = write (fd[1], &buf, 1);

        if (r != 1)
                return -1;
        return 0;
}
