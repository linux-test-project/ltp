/* The following functions are used to synchronize father and sons processes.
 * 
 * create_sync_pipes: create pipes used for the synchronization. Must be done
 *                    by father process before a fork.
 *
 * wait_son_startup: wait a son process to reach the "notify_startup" function.
 *
 * notify_startup: notify the father process a son has started its execution.
 */
int sync_pipe_create( int fd[]);
int sync_pipe_wait( int fd[]);
int sync_pipe_notify( int fd[]);
