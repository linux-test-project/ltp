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
 * process. This option may not be usefull to stress.
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

int writeToAllClients(char *foo);//#define DEBUG

int serverReceiveNet();
int clientReceiveNet();
int serverReceiveClient(int n);
int setupClients(int type, char *fname, int nThread);
int serverCloseConnection();
int getConfiguration(int *type, char *fname, int *nThread);
int readFromServer(char *message);
int serverSendClient(int n);


enum etat_t     {
                CLEAN,
                RDONLY,
                RESULTAT,
                WRONLY,
                SELECT,
                LOCK,
                SYNC,
                FIN,
                READLOCK,
                WRITELOCK,
                BYTELOCK,
                BYTELOCK_READ,
                BYTELOCK_WRITE
};

/* Donnees communes aà tous les processu */
/* Public data */
struct donneesPub {
    /* Nombre de clients */
    /* Number of clients */
    int nclnt;
    /* Liste des clients (liste des tubes)*/
    /* List of master to slave pipes */
    int **lclnt;
    /* Tube de communication avec le maitre */
    /* Slave to master pipe */
    int maitre[2];
    /* Liste des threads */
    /* Thread list */
    pthread_t *lthreads;
    /* nom du fichier test */
    /* test file name */
    char *fname;
    /* descripteur du fichier test */
    /* test file file-descriptor */
    int fd;
    /* Affichage des messages d'erreur */
    /* Detailed error messages */
    int verbose;
};

/* Donnees privees aux processus */
/* private data */
struct donneesPriv {
    /* Numero de thread. */
    /* thread number */
    int whoami;
};

struct donneesFils{
    struct donneesPub *dp;
    struct donneesPriv *dpr;
};


struct s_test {
    int test;
    int type;
    char *nom;
    int resAtt;

};




int configureServeur(int  max);
int configureClient(char *s);

#endif
