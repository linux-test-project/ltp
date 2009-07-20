/* The following functions are used to synchronize father and sons processes.
 *
 * create_sync_pipes: create pipes used for the synchronization. Must be done
 *                    by father process before a fork.
 *
 * wait_son_startup: wait a son process to reach the "notify_startup" function.
 *
 * notify_startup: notify the father process a son has started its execution.
 */
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

/* fifo_name is used to create named pipe. NULL means anonymous pipe. */
#define PIPE_NAME	NULL
int sync_pipe_create( int fd[], const char *pipe_name);
int sync_pipe_close(int fd[], const char *pipe_name);
int sync_pipe_wait( int fd[]);
int sync_pipe_notify( int fd[]);

