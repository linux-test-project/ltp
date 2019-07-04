/* *************************************************
 * *********** README ******************************
 * *************************************************
 *
 * COMPILE : make
 * RUN : ./locktests -n <number of concurent process> -f <test file> [-P]
 *
 * GOAL : This test tries to stress the fcntl locking functions.  A
 * master process sets a lock on a file region (this is called "byte
 * range locking").  Some slave processes try to perform operations on
 * this region, such as read, write, set a new lock ... The expected
 * results of these operations are known.  If the operation's result is
 * the same as the expected one, the test suceeds, else it fails.
 *
 * Slaves are either concurent processes or threads.
 * -n <num>  : Number of threads to use (mandatory).
 * -f <file> : Run the test on a test file defined by the -f option (mandatory).
 * -T        : Use threads instead of processes (optional).
 *
 * HISTORY : This program has been written to stress NFSv4 locks. -P
 * option was created to verify NFSv4 locking was thread-aware, and so,
 * locking behaviour over NFSv4 was POSIX correct both using threads and
 * process. This option may not be useful to stress.
 *
 * EXAMPLE : ./locktests -n 50 -f /file/system/to/test
 *
 *
 * Vincent ROQUETA 2005 - vincent.roqueta@ext.bull.net
 * BULL S.A.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#ifdef STDARG
#include <stdarg.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/times.h>
#ifdef MMAP
#include <sys/mman.h>
#endif
#include <inttypes.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#ifdef O_SYNC
#define OPENFLAGS       (O_CREAT | O_RDWR | O_SYNC )
#else
#define OPENFLAGS       (O_CREAT | O_RDWR )
#endif
#define OPENMODES       (0600)
#define MANDMODES       (0600)
/*(02666)*/

#define SUCCES 1
#define ECHEC  0

#define TRUE 1
#define FALSE 0

#define PROCESS 0
#define THREAD 1


//#define DEBUG
#ifdef DEBUG
        #define E(a)  perror(a)
        #define P(a,b) printf(a,b)
#else
        #define E(a)
        #define P(a,b)
#endif



#ifndef LOCKTESTS_H
#define LOCKTESTS_H

#define M_SIZE 512

int writeToAllClients(char *foo);

int serverReceiveNet();
int clientReceiveNet();
int serverReceiveClient(int n);
int setupClients(int type, char *fname, int nThread);
int serverCloseConnection();
int getConfiguration(int *type, char *fname, int *nThread);
int readFromServer(char *message);
int serverSendClient(int n);


enum state_t     {
                CLEAN,
                RDONLY,
                RESULT,
                WRONLY,
                SELECT,
                LOCK,
                SYNC,
                END,
                READLOCK,
                WRITELOCK,
                BYTELOCK,
                BYTELOCK_READ,
                BYTELOCK_WRITE
};

/* Public data */
struct dataPub {
    /* Number of clients */
    int nclnt;
    /* List of master to slave pipes */
    int **lclnt;
    /* Slave to master pipe */
    int master[2];
    /* Thread list */
    pthread_t *lthreads;
    /* test file name */
    char *fname;
    /* test file file-descriptor */
    int fd;
    /* Detailed error messages */
    int verbose;
};

/* private data */
struct dataPriv {
    /* thread number */
    int whoami;
};

struct dataChild{
    struct dataPub *dp;
    struct dataPriv *dpr;
};


struct s_test {
    int test;
    int type;
    char *nom;
    int resAtt;

};




int configureServer(int  max);
int configureClient(char *s);

#endif
