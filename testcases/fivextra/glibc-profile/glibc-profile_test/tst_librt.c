#include <aio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BYTES 8

int test_rt ( void )
{
    int r;
    int fildes;
    struct aiocb cb;
    char out[BYTES] = "0123456";
    char in[BYTES];
    
    if ((fildes = open( "./test_rt", O_WRONLY|O_CREAT, 0600 )) < 0) {
        perror( "opening file" ); return 1;
    }

    cb.aio_fildes = fildes;
    cb.aio_offset = 0;
    cb.aio_buf = out;
    cb.aio_nbytes = BYTES;
    cb.aio_reqprio = 0;
    cb.aio_sigevent.sigev_notify = SIGEV_NONE;

    /* write some bytes to the O_WRONLY file descriptor */
    r = aio_write( &cb );
    while (aio_error( &cb ) == EINPROGRESS) { usleep( 10 ); }
    if (aio_return( &cb ) != BYTES) { perror( "write" ); return 1; }


    return 0;
}

int main (void)
 {

   return test_rt();
 }
