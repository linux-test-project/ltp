/* The following functions are used to synchronize father and sons processes.
 * 
 * create_sync_pipes: create pipes used for the synchronization. Must be done
 *                    by father process before a fork.
 *
 * wait_son_startup: wait a son process to reach the "notify_startup" function.
 *
 * notify_startup: notify the father process a son has started its execution.
 */
int create_sync_pipes(int fd[]);
int wait_son_startup (int fd[]);
int notify_startup (int fd[]);
