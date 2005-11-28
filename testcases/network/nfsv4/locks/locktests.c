/* *************************************************
 * *********** README ****************************** 
 * ************************************************* 
 * 
 * COMPILE : make 
 * RUN : ./locktests -n <number of concurent process> -f <test file> [-P]
 * 
 * GOAL : This tests try to stress fcntl locking functions. A master process set a lock on a file region (byte range locking).
 * Some slaves process tries to perform operations on this region, like read, write, set a new lock ... Expected results of this 
 * operations are known. If the operation result is the same as the expected one, the test sucess, else it fails.
 *
 * Slaves are concurent processes or thread. 
 * -n : Number of thread is defined by the -n option (mandatory). 
 * -f : The test is ran on a test file defined by the -f option (mandatory).
 * -P : Use pthreads instead of forked processes (optional). Result table is different.
 *
 * HISTORY : This program has been written to stress NFSv4 locks. -P option was created to verify NFSv4 locking was thread-aware,
 * and so, locking behaviour over NFSv4 was POSIX correct both using threads and process. This option may not be usefull to stress.
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
//#define P(a,b) printf(a,b)


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
                

int MAXLEN=64;
int MAXTEST=10;
/* Quel lock sera apppliqué par le maitre en début de test */
int LISTE_LOCKS[]=      {READLOCK, WRITELOCK,
                         READLOCK, WRITELOCK,
                         READLOCK, WRITELOCK,
                         READLOCK, WRITELOCK,
                         BYTELOCK_READ, BYTELOCK_WRITE
};

/* Les opérations que les programes esclaves essaieront de faire */
int LISTE_TESTS[]=      {WRONLY,  WRONLY,
                         RDONLY,  RDONLY,
                         READLOCK, WRITELOCK,
                         WRITELOCK, READLOCK,
                         BYTELOCK_READ, BYTELOCK_WRITE
};


char *LISTE_NOMS_TESTS[]={"WRITE ON A READ  LOCK",
                          "WRITE ON A WRITE LOCK",
                          "READ  ON A READ  LOCK",
                          "READ  ON A WRITE LOCK",
                          "SET A READ  LOCK ON A READ  LOCK",
                          "SET A WRITE LOCK ON A WRITE LOCK",
                          "SET A WRITE LOCK ON A READ  LOCK",
                          "SET A READ  LOCK ON A WRITE LOCK",
                          "READ LOCK THE WHOLE FILE BYTE BY BYTE",
                          "WRITE LOCK THE WHOLE FILE BYTE BY BYTE"

};


/* Resultat attendu du test */

int LISTE_RESULTATS_PROCESS[]=  {SUCCES, SUCCES, 
                                SUCCES,  SUCCES,
                                SUCCES,  ECHEC,
                                ECHEC,   ECHEC,
                                SUCCES,  SUCCES

};

int LISTE_RESULTATS_TREADS[]=  { SUCCES,  SUCCES, 
                                SUCCES,  SUCCES,
                                SUCCES,  SUCCES,
                                SUCCES,  SUCCES,
                                ECHEC,  ECHEC

};

int *LISTE_RESULTATS=NULL;
char *eType=NULL;

int TOTAL_RESULTAT_OK[]={0,0,0,0,0,0,0,0,0,0};



void * esclave(void *donnees);

/* Données communes à tous les processus */
struct donneesPub {
    /* Nombre de clients */
    int nclnt;
    /* Liste des clients (liste des tubes)*/
    int **lclnt;
    /* Tube de communication avec le maitre */
    int maitre[2];
    /* Liste des threads */
    pthread_t *lthreads;
    /*nom du fichier test*/
    char *fname;
    /*descripteur du fichier test*/
    int fd;
    /* Affichage des messages d'erreur */
    int verbose;
};
 
/* Données privées aux processus */
struct donneesPriv {
    /* Numero de thread. */
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



struct donneesFils *initClientPThread(int i, struct donneesPub *dp)
{

    struct donneesPriv *dpr;    
    struct donneesFils *df;

    /*Initialisation des champs de données */
    dpr=(struct donneesPriv *)malloc(sizeof(struct donneesPriv ));
    df=(struct donneesFils *)malloc(sizeof(struct donneesFils));
    dpr->whoami=i;
    df->dp=dp;
    df->dpr=dpr;
    /* Initialisation du tubes de synchronisation */
    dp->lclnt[i]=(int *)malloc(sizeof(int)*2);
    if(pipe(dp->lclnt[i])<0){
        perror("Impossible to create pipe\n");
        exit(1);
    }
    write(0,".",1); 
    /* Initialisation et lancement du thread */
    if(pthread_create(&(dp->lthreads[i]), NULL, (void *)&esclave, (void *)df)<0){
        perror("Impossible to create thread num ");
        printf("%d\n", dpr->whoami);
        exit(1);
    }
    return df;
}
int (*terminer)(int a);



int terminerThread(int a){
    pthread_exit(&a);
}
int terminerProcess(int a){
    exit(a);
}
struct donneesFils *initClientFork(int i, struct donneesPub *dp)
{

    struct donneesPriv *dpr;    
    struct donneesFils *df;
 
    /*Initialisation des champs de données */
    dpr=(struct donneesPriv *)malloc(sizeof(struct donneesPriv ));
    df=(struct donneesFils *)malloc(sizeof(struct donneesFils));
    dpr->whoami=i;
    df->dp=dp;
    df->dpr=dpr;
    /* Initialisation du tubes de synchronisation */
    dp->lclnt[i]=(int *)malloc(sizeof(int)*2);
    if(pipe(dp->lclnt[i])<0){
        perror("Impossible to create pipe\n");
        exit(1);
    }
    P("Initialization %d\n", i);
    write(0,".",1); 
    return df;
}

int initTest(struct donneesPub *dp){
   
    P("Maitre ouvre %s\n",dp->fname);
    dp->fd = open(dp->fname, OPENFLAGS, MANDMODES);
    if (dp->fd < 0) {
        perror("lock test : can't open test file :");
        terminer(1);
    }
    P("fd=%d\n", dp->fd);
    return 0;
}

int (*load)(struct donneesPub *dp);


int initialise(int clnt, struct donneesPub *dp){
 
    /* Initialisation des données publiques */
    printf("Init\n");
    dp->nclnt=clnt;
    dp->lclnt=(int **)malloc(sizeof(int *)*clnt);
    dp->lthreads=(pthread_t *)malloc(sizeof(pthread_t)*clnt);
   
    /* initialisation de la communication avec le maitre */
    if(pipe(dp->maitre)<0){
        perror("Master pipe creation error\n");
        exit(1);
    }
    printf("%s initalization\n",eType);
    load(dp);
    initTest(dp);
    
    
    return 0;
}

    

        
int loadThread(struct donneesPub *dp){
    int i;
   
    /* Initialisation avec des threads */
    for(i=0; i<dp->nclnt; i++){
        write(0,".",1);
        initClientPThread(i, dp);
    }
    return 0;
}

int loadProcess(struct donneesPub *dp){
    int i;
    struct donneesFils *df;
    for(i=0; i<dp->nclnt; i++){
            df=initClientFork(i,dp);
        if (!fork() ) {
            P("Running slave num: %d\n",df->dpr->whoami);
            write(0,".",1);
            esclave((void *)df);
            exit(0);
        }
    }
    return 0;
}

int testSuiv(int n){
   return LISTE_TESTS[n];
}
int resAttSuiv(int n){
    return LISTE_RESULTATS[n];
}
char * nomTestSuiv(int n){
    return LISTE_NOMS_TESTS[n];
}
int lockSuiv(int n){
    return LISTE_LOCKS[n];
}

int matchResult(int r, int n){
   
    P("r=%d\n",r);
    if(r==LISTE_RESULTATS[n])
        return 1;
    else return 0;
}


void compteur(int r,int n){
    TOTAL_RESULTAT_OK[n]+=matchResult(r, n);
}


void validationResultats(int n, struct donneesPub *dp){
    int i,u,l;
    struct flock request;
    
    TOTAL_RESULTAT_OK[n]=0;
    l=FALSE;
    u=TRUE;
    /* Si le résultat de l'opération attendu est un succéson prévoi d'incrémenté le nombre de résultats corrects */
    if(LISTE_RESULTATS[n]){
        l=TRUE;
        u=FALSE;
    }
            
    for(i=0;i<dp->nclnt;i++){
        request.l_type=F_WRLCK; 
        request.l_whence=SEEK_SET;
        request.l_start=i;
        request.l_len=1;
        fcntl(dp->fd, F_GETLK, &request);
        /* On vérifie si le lock est mis ou non */
        if(request.l_type!=F_UNLCK)
            TOTAL_RESULTAT_OK[n]+=l;
        else
            TOTAL_RESULTAT_OK[n]+=u;
    }
}

void lockWholeFile(struct flock *request){
    request->l_whence = SEEK_SET;
    request->l_start =  0;
/* Lock de l'ensemble du fichier*/
    request->l_len = 0;
}     


void selectTest(int n, struct s_test *test){
    
    test->test=testSuiv(n);
    test->resAtt=resAttSuiv(n);
    test->nom=nomTestSuiv(n);
    test->type=lockSuiv(n);
}

void rapport(clnt){
    int i;
    printf("\n%s number : %d\n", eType, clnt);
    printf("%s number running test successfully :\n",eType);
    for(i=0; i<MAXTEST; i++)
        printf("%d %s of %d successfully ran test : %s\n", TOTAL_RESULTAT_OK[i], eType, clnt, LISTE_NOMS_TESTS[i]);
    
}


void maitre(struct donneesPub *dp)
{
    int i,n,a,bl;
    int clnt, reception;
    int r;
    char tmp[MAXLEN], *buf;
    struct flock request;
    struct s_test tLock;
    enum etat_t etat;
    
    char phraseTest[]="Ceci est une phrase test écrite par le maitre dans le fichier";   
    clnt=dp->nclnt;
    reception=dp->maitre[0];
    etat=SELECT;
    /* On commence par le premier test */
    n=0;
    printf("\n--------------------------------------\n");
    while(1){
        switch(etat){
            case SELECT:
                /* Selection du test à effectuer*/
                printf("\n");
                E("Maitre: SELECT");
                selectTest(n, &tLock);
                etat=tLock.type;
                bl=0;               
                if(n<MAXTEST){
                    memset(tmp,0,MAXLEN);
                    sprintf(tmp,"TEST : TRY TO %s:",LISTE_NOMS_TESTS[n]);
                    write(0, tmp, strlen(tmp));
                }
                else
                    etat=FIN;
                P("etat=%d\n", etat);
                n+=1;
                continue;

            case RDONLY: /* Not reached */
            case WRONLY: /* Not reached */


            case READLOCK:
                P("Read lock :%d\n", etat);
                request.l_type=F_RDLCK;
                etat=LOCK;
                continue;
                
            case WRITELOCK:
                P("Write lock :%d\n", etat);
                request.l_type=F_WRLCK;
                etat=LOCK;
                continue;

            case LOCK:
                /* On applique le lock que l'on veut */
                E("Maitre: LOCK");
                write(dp->fd,phraseTest,strlen(phraseTest));
                lockWholeFile(&request);
                if(fcntl(dp->fd, F_SETLK, &request)<0){
                    perror("Master: can't set lock\n");
                    perror("Echec\n");
                    exit(0);
                }
                sleep(1);
                E("Maitre");
                etat=SYNC;
                continue;

            case BYTELOCK_READ:
                bl=1;
                request.l_type=F_RDLCK;
                etat=SYNC;
                continue;
                
            case BYTELOCK_WRITE:
                bl=1;
                request.l_type=F_WRLCK;
                etat=SYNC;
                continue;
                
            case BYTELOCK:
                /* L'idée est de faire locker l'ensemble du fichier octet par octet par un ensemble de sous processus
                 * Il nous faut donc 
                 * -créer un fichier ayant autant d'octet que le nombre de processus passé en paramètres
                 * -passer les sections à locker à chacun des esclaves
                 * -vérifier que les locks sont bien appliqués
                 * */
                
                /* On crée une chaine de caractères à enregistrer dans le fichier qui contienne exactement un octet par
                 * processus.
                 */
                P("Maitre: BYTELOCK: %d\n", etat);
                buf=(char *)malloc(clnt);
                memset(buf,'*', clnt);
                write(dp->fd, buf, clnt);
                request.l_whence=SEEK_SET;
                
                /* Chaque processus esclave reécrit son champs à locker. */
                for(i=0;i<clnt;i++){
                    /* On renseigne la structure avec le lock qui va bien */
                    request.l_start=i;
                    request.l_len=1;
                    write(dp->lclnt[i][1], &request, sizeof(struct flock));
                }                    
                etat=RESULTAT;
                continue;


            case SYNC:
                /* Synchronisation des processus esclaves. */
                P("Maitre: SYNC %d\n", etat);
                
                /* On configure les esclaves pour le test */
                for(i=0; i<clnt; i++)
                    write(dp->lclnt[i][1], &(tLock.test), sizeof(int));
                if(bl){
                    etat=BYTELOCK;
                    continue;
                }
                
                if(n<MAXTEST+1) etat=RESULTAT;
                else etat=FIN;
                continue;

            case RESULTAT:
                /* On lit les résultats un par un */
                E("Maitre: RESULTAT");
                for(i=0; i<clnt; i++){
                    if((a=read(reception, &r,sizeof(int)))<0){
                        perror("Can't read master pipe");
                        exit(1);
                    }
                    compteur(r,n-1);

                }
                if(bl) validationResultats(n-1, dp);
                etat=CLEAN;
                continue;

            case CLEAN:
                a=CLEAN;
                for(i=0; i<clnt; i++)
                    write(dp->lclnt[i][1], &a, sizeof(int));
                /* Get CLEAN AcK from slaves */
                for(i=0; i<clnt; i++){
                    if(read(reception, &a,sizeof(int))<0){
                        perror("Can't read master pipe");
                        exit(1);
                    }
                } 
                
                /* close and open file */
                close(dp->fd);
                initTest(dp);
                etat=SELECT;
                continue;
            case FIN:
                a=FIN;
                for(i=0; i<clnt; i++)
                    write(dp->lclnt[i][1], &a, sizeof(int));
                break;
                
                printf("(end)\n");
                exit(0);

        }/* switch */
        break;
    }/* while */
    
    rapport(clnt);
}






void * esclave(void *donnees)
{
    struct donneesFils *df;
    int i, cmd,a, resultat,ftest;
    int maitre;
    struct flock request;
    char tmp[16];
    char *phraseTest="L'écriture a réussi";
    int len;
    enum etat_t etat;
    
    len=strlen(phraseTest);
    df=(struct donneesFils *)donnees; 
    i=df->dpr->whoami;
    P("Escalve n=%d\n", i);
    cmd=df->dp->lclnt[i][0];
    maitre=df->dp->maitre[1];
    etat=SYNC;
    errno=0;
    memset(tmp,0,16);
    while(1){
        switch(etat){
            case SELECT: /* NOT reached */
            case SYNC:
                if(read(cmd,&etat,sizeof(int))<0){
                    perror("Lecture du pipe \n");
                    exit(1);
                }
                P("State :%d\n", etat);
                continue;
            case RDONLY:
                /* On tente de lire un fichier */
                P("TEST READ ONLY %d\n", RDONLY);
                if((ftest=open(df->dp->fname, O_RDONLY|O_NONBLOCK))<0){
                    resultat=ECHEC;
                    etat=RESULTAT;
                    if(df->dp->verbose)
                        perror("Open:");
                    continue;
                }
                P("fd=%d\n",ftest);
                a=read(ftest, tmp, 16);
                if(a<16)
                    resultat=ECHEC;
                else
                    resultat=SUCCES;
                etat=RESULTAT;
                continue;
                        
            case WRONLY:
                /* On tente de lire un fichier */
                P("TEST WRITE ONLY %d\n", WRONLY);
                if((ftest=open(df->dp->fname, O_WRONLY|O_NONBLOCK))<0){
                    resultat=ECHEC;
                    etat=RESULTAT;
                    if(df->dp->verbose)
                        perror("Open read only:");
                    continue;
                }
                P("fd=%d\n",ftest);
                if(write(ftest, phraseTest, len)<len)
                    resultat=ECHEC;
                else
                    resultat=SUCCES;
                etat=RESULTAT;
                continue;

            case LOCK: /* NOT reached */
                
                
            case READLOCK:
                /* On essaie de lire une section lockée en lecture sur le fichier */
                P("READ LOCK %d\n", F_RDLCK);
                if((ftest=open(df->dp->fname, O_RDONLY|O_NONBLOCK))<0){
                    resultat=ECHEC;
                    etat=RESULTAT;
                    if(df->dp->verbose)
                        perror("Open read-write:");
                    continue;
                }

                P("fd=%d\n",ftest);
                /* Lock de l'ensemble du fichier*/
                request.l_type=F_RDLCK;
                request.l_whence = SEEK_SET;
                request.l_start =  0;
                request.l_len = 0;
                
                if(fcntl(ftest, F_SETLK, &request)<0){
                    if(df->dp->verbose || errno != EAGAIN)
                        perror("RDONLY: fcntl");
                    resultat=ECHEC;
                }
                else
                    resultat=SUCCES;
                etat=RESULTAT;
                continue;

            case WRITELOCK:
                /* On essaie d'écrire  le fichier */
                P("WRITE LOCK %d\n", F_WRLCK);
                if((ftest=open(df->dp->fname, O_WRONLY|O_NONBLOCK))<0){
                    resultat=ECHEC;
                    etat=RESULTAT;
                    if(df->dp->verbose)
                        perror("\nOpen:");
                    continue;
                }
                /* Lock de l'ensemble du fichier*/
                P("fd=%d\n",ftest);
                request.l_type=F_WRLCK;
                request.l_whence = SEEK_SET;
                request.l_start =  0;
                request.l_len = 0;
                
                if(fcntl(ftest, F_SETLK, &request)<0){
                    if(df->dp->verbose || errno != EAGAIN)
                        perror("\nWRONLY: fcntl");
                    resultat=ECHEC;
                }
                else                    
                    resultat=SUCCES;
                etat=RESULTAT;
                continue;

            case BYTELOCK_READ:
                etat=BYTELOCK;
                continue;

            case BYTELOCK_WRITE:
                etat=BYTELOCK;
                continue;
                
            case BYTELOCK:
                /* On se met en attente de la section à locker. L'ensemble de la structure est 
                 * donnée par le maitre et transmise par le tube.
                 */
                P("WRITE LOCK %d\n", etat);
                if(read(cmd, &request, sizeof(struct flock))<0){
                    perror("Pipe : read error");
                    terminer(1);
                }

                if((ftest=open(df->dp->fname, O_RDWR|O_NONBLOCK))<0){
                    resultat=ECHEC;
                    etat=RESULTAT;
                    if(df->dp->verbose)
                        perror("\nOpen:");
                    continue;
                }
                if(fcntl(ftest, F_SETLK, &request)<0){
                    if(df->dp->verbose || errno != EAGAIN)
                        perror("\nBTLOCK: fcntl");
                    resultat=ECHEC;
                    etat=RESULTAT;
                    continue;
                }
                /* On change le caractère à la place donnée pour vérification */
                a=lseek(ftest, request.l_start,SEEK_SET);
                write(ftest,"=",1);
                resultat=SUCCES;
                etat=RESULTAT;
                continue;
                 
            case RESULTAT:
                if(resultat==SUCCES)
                    write(0,"=",1);
                else
                    write(0,"x",1);
                P("RESULTAT: %d\n", resultat);
                if(write(maitre,&resultat,sizeof(int))<0){
                    P("Slave num %d\n", df->dpr->whoami);
                    perror("Slave\n");
                }
                
                etat=SYNC;
                continue;

            case CLEAN:
                close(ftest);
                /* Send CLEAN Ack */
                if(write(maitre,&resultat,sizeof(int))<0){
                    P("Slave num %d\n", df->dpr->whoami);
                    perror("Slave\n");
                }
                etat=SYNC;
                continue;
            
            
            case FIN:
                E("(End)\n");
                terminer(0);
            printf("Unknown command\n");
            terminer(1);

        }
    }



}

char *nextArg(int argc, char **argv, int *i){
    if(((*i)+1)<argc){
            (*i)+=1;
            return argv[(*i)];
        }
        return NULL;
}
        
int usage(){
        printf("locktest -n <number of process> -f <test file> [-T]\n");
        printf("-T : Use threads instead of process (First designed to test differences between thread and process locking behaviour over NFS)\n");
        printf("Number of child process must be higher than 1\n");
        printf("\n");
        printf("Send bugs to vincent.roqueta@ext.bull.net\n");
        exit(0);
}
    


int main(int argc,char ** argv){
    struct donneesPub dp;
    int i, nThread=0;
    char *tmp;
    int type=0;
    
    type=PROCESS;
    dp.fname=NULL;
    dp.verbose=0;

    for(i=1;i<argc;i++){
        
        if(!strcmp("-n",argv[i])){
              if(!(tmp=nextArg(argc, argv,&i)))
                  usage();
              nThread=atoi(tmp);
              continue;
        }

        if(!strcmp("-f",argv[i])){
            if(!(dp.fname=nextArg(argc, argv,&i)))
                usage();
            continue;
        }
        if(!strcmp("-T",argv[i])){
            type=THREAD;
            continue;
        }
        if(!strcmp("-v",argv[i])){
            dp.verbose=TRUE;
            continue;
        }
        /* Option inconnue */
        printf("Ignoring unknown option: %s\n", argv[i]);

        
    }
    if(!(dp.fname && nThread))
            usage();
    
    
    if(type==THREAD){
        if(dp.verbose)
            printf("By thread\n");
        load=loadThread;
        terminer=terminerThread;
        eType="thread";
        LISTE_RESULTATS=LISTE_RESULTATS_TREADS;
    }else{
        if(dp.verbose)
            printf("By process. Use -T to test with multithreading\n");
        load=loadProcess;
        eType="process";
        terminer=terminerProcess;
        LISTE_RESULTATS=LISTE_RESULTATS_PROCESS;
    }
    
    initialise(nThread,&dp);
    maitre(&dp);
    return 0;
}





