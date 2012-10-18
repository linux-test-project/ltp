#include "locktests.h"

#include <netdb.h>
#include <string.h>
#define PORT 12346
#define MAX_CONNECTION 16

int maxClients;
int *fdClient;
char *serveur;
int fdServeur;
extern char message[M_SIZE];

int serverReceiveClient(int c) {
    char tmp[M_SIZE];
    int r,s;
    /* Il faut etre sur que l'on lit _exactement_ la trame envoyee (M_SIZE) */
    /* Ensure we read _exactly_ M_SIZE characters in the message */
    memset(message,0,M_SIZE);
    memset(tmp,0,M_SIZE);
    r=0;
    s=0;

    while (s<M_SIZE) {
        r=read(fdClient[c],tmp, M_SIZE-s);
        /* On complete le message au fur et a mesure */
        /* Loop until we have a complete  message */
        strncpy(message+s,tmp,r);
        s+=r;
    }
   return s;
}

int serverSendClient(int n) {
    return write(fdClient[n],message, M_SIZE);
}

int clientReceiveNet() {
    readFromServer(message);
        return 0;
}

int setupConnectionServeur() {
    struct sockaddr_in local;
    int c;
    socklen_t size;
    int sock;
    struct sockaddr_in remote;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0))<0) {
        perror ("socket");
        exit(1);
    }
    memset(&local, 0x00, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = INADDR_ANY;
    memset(&(local.sin_zero), 0x00, 8);

    if (bind(sock, (struct sockaddr *)&local, sizeof(struct sockaddr))== -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sock, MAX_CONNECTION) == -1) {
        perror("listen");
        return 1;
    }
    size = sizeof(struct sockaddr_in);
    for (c=0;c<maxClients; c++) {

        /* On accepte les connections clientes */
        /* Accept incoming connections */
        if ((fdClient[c]=accept(sock, (struct sockaddr *)&remote, &size)) == -1) {
            perror("accept");
            return 1;
        }

    }
        return 0;
}

int writeToClient(int c, char *message) {
    return write(fdClient[c], message, 512);
}

int serverCloseConnection() {
    int c;
    for (c=0;c<maxClients;c++)
        close(fdClient[c]);
        return 0;

}

int writeToAllClients(char *foo) {
    int c;
    for (c=0;c<maxClients;c++)
        writeToClient(c, foo);
        return 0;
}

int setupClients(int type, char *fname, int nThread) {
    /*
     * Envoi des parametres a tous les clients
     *
     * on doit envoyer 3 parametres :
     * - l'emplacement du fichier test
     * - Le nombre d'esclaves
     * - Le type de sous processus : thread ou process
     */

    /*
     * Send parameters to all slaves :
     *
     * We must send :
     * - the position of the test file
     * - the number of slaves for each client
     * - The kind of slaves : process or thread
     */

    char message[512];
    sprintf(message,"%d:%s:%d::", type,fname,nThread);
    writeToAllClients(message);
        return 0;
}

int configureServeur(int  max) {
    maxClients=max;
    fdClient=(int *)malloc(sizeof(int)*max);

    setupConnectionServeur();

        return 0;
}

int setupConnectionClient() {

    struct hostent *server;
    struct sockaddr_in serv_addr;

    if (! (server=gethostbyname(serveur))) {
        printf("erreur DNS\n");
        return 1;
    }

    fdServeur = socket (AF_INET, SOCK_STREAM, 0);
    if (fdServeur < 0) {
        perror ("socket");
        return 1;
    }

    serv_addr.sin_addr = *(struct in_addr*)server->h_addr;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_family = AF_INET;
    if (connect(fdServeur, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }
 return 0;
}

int readFromServer(char *message) {
    char tmp[M_SIZE];
    int r,s;
    /* Il faut etre sur que l'on lit _exactement_ la trame envoyee de taille M_SIZE */
    /* Ensure we read exactly M_SIZE characters */
    memset(message,0,M_SIZE);
    memset(tmp,0,M_SIZE);
    r=0; s=0;
    while (s<M_SIZE) {
        r=read(fdServeur,tmp, M_SIZE-s);
        /* On complete le message au fur et a mesure */
        /* Loop until we have a complete message */
        strncpy(message+s,tmp,r);
        s+=r;
    }
   return s;
}

int getConfiguration(int *type, char *fname, int *nThread) {
    char conf[M_SIZE];
    char *p;
    int i;
    readFromServer(conf);
    p=strtok(conf,":");
    printf("%s\n",p);
    *type=atoi(p);
    p=strtok(NULL,":");
    i=strlen(p);
    strncpy(fname, p,i);
    p=strtok(NULL,":");
    *nThread=atoi(p);

        return 0;
}

int configureClient(char *s) {
    serveur=s;
    setupConnectionClient();
        return 0;
}
