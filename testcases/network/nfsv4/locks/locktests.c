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
 * results of these operations are known.  If the operation result is
 * the same as the expected one, the test suceeds, else it fails.
 *
 *
 *
 * Slaves are concurent processes or thread.
 * -n <num>  : Number of threads to use (mandatory).
 * -f <file> : Run the test on given test file defined by the -f option (mandatory).
 * -c <num>  : Number of clients to connect before starting the tests.
 *
 * HISTORY : This program was written to stress NFSv4 locks.
 * EXAMPLE : ./locktests -n 50 -f /file/system/to/test
 *
 *
 * Vincent ROQUETA 2005 - vincent.roqueta@ext.bull.net
 * BULL S.A.
 */

#include "locktests.h"

int MAXLEN = 64;
int MAXTEST = 10;
extern int maxClients;
extern int fdServeur;

char message[M_SIZE];
int esclaveLecteur;
int maitreLecteur;
int esclaveEcrivain;

/* Quel lock sera appplique par le maitre en debut de test */
/* Which lock will be applied by the master process on test startup */
int LISTE_LOCKS[] = { READLOCK, WRITELOCK,
	READLOCK, WRITELOCK,
	READLOCK, WRITELOCK,
	READLOCK, WRITELOCK,
	BYTELOCK_READ, BYTELOCK_WRITE
};

/* Les operations que les programes esclaves essaieront de faire */
/* The operations the slave processes will try to perform */

int LISTE_TESTS[] = { WRONLY, WRONLY,
	RDONLY, RDONLY,
	READLOCK, WRITELOCK,
	WRITELOCK, READLOCK,
	BYTELOCK_READ, BYTELOCK_WRITE
};

/* List of test names */
char *LISTE_NOMS_TESTS[] = { "WRITE ON A READ  LOCK",
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

/* Resultat attendu du test - Processus */
/* List of expected test results, when slaves are processes */

int LISTE_RESULTATS_PROCESS[] = { SUCCES, SUCCES,
	SUCCES, SUCCES,
	SUCCES, ECHEC,
	ECHEC, ECHEC,
	SUCCES, SUCCES
};

/* List of expected test results, when slaves are threads */

int LISTE_RESULTATS_THREADS[] = { SUCCES, SUCCES,
	SUCCES, SUCCES,
	SUCCES, SUCCES,
	SUCCES, SUCCES,
	ECHEC, ECHEC
};

int *LISTE_RESULTATS = NULL;
char *eType = NULL;

int TOTAL_RESULTAT_OK[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void *esclave(void *donnees);
int (*terminer) (int a);

int terminerProcess(int a)
{
	exit(a);
}

int (*load) ();

struct donneesPub dp;

/* Manipulation des tests/ noms de tests/ resultats de tests */
/* Functions to access tests/tests names/tests results*/
int testSuiv(int n)
{
	return LISTE_TESTS[n];
}

int resAttSuiv(int n)
{
	return LISTE_RESULTATS[n];
}

char *nomTestSuiv(int n)
{
	return LISTE_NOMS_TESTS[n];
}

int lockSuiv(int n)
{
	return LISTE_LOCKS[n];
}

/* Verifie si le resultat obtenu pour le test est le resultat attendu pour ce test */
/* Verifie the test result is the expected one */
int matchResult(int r, int n)
{

	P("r=%d\n", r);
	if (r == LISTE_RESULTATS[n])
		return 1;
	else
		return 0;
}

/* Incremente le nombre de process ayant valide le test */
/* Increments the number of process which have successfully passed the test */
void compteur(int r, int n)
{
	TOTAL_RESULTAT_OK[n] += matchResult(r, n);
}

/* Validation des resultats pour le lock de fichier octet par octet
 * Pour chaque octet on verifie qu'un lock de 1 octet est bien pose
 */
/* Special case for test 'lock file byte byte by byte'.
 * We ensure each byte is correctly locked.
 */

void validationResultats(int n)
{
	int i, u, l, fsize;
	struct flock request;

	fsize = dp.nclnt * (maxClients + 1);
	TOTAL_RESULTAT_OK[n] = 0;
	l = FALSE;
	u = TRUE;
	/* Si le resultat de l'operation attendu est un succe on prevoi d'incrementer le nombre de resultats corrects */
	/* If the expected operation result is a success, we will have to increase the number of correct results */
	if (LISTE_RESULTATS[n]) {
		l = TRUE;
		u = FALSE;
	}

	for (i = 0; i < fsize; i++) {
		request.l_type = F_WRLCK;
		request.l_whence = SEEK_SET;
		request.l_start = i;
		request.l_len = 1;
		fcntl(dp.fd, F_GETLK, &request);
		/* On verifie si le lock est mis ou non */
		/* Ensure the lock is correctly set */
		if (request.l_type != F_UNLCK)
			TOTAL_RESULTAT_OK[n] += l;
		else
			TOTAL_RESULTAT_OK[n] += u;
	}
}

/* Procedures d'initialisation */
/* Initialisation functions */

int initTest()
{

	P("Maitre ouvre %s\n", dp.fname);
	dp.fd = open(dp.fname, OPENFLAGS, MANDMODES);
	if (dp.fd < 0) {
		perror("lock test : can't open test file :");
		terminer(1);
	}
	P("fd=%d\n", dp.fd);
	return 0;
}

struct donneesFils *initClientFork(int i)
{
	struct donneesPriv *dpr;
	struct donneesFils *df;

	/* Initialisation des champs de donnees */
	/* Initialize private data fields */
	dpr = malloc(sizeof(struct donneesPriv));
	df = malloc(sizeof(struct donneesFils));
	dpr->whoami = i;
	df->dp = &dp;
	df->dpr = dpr;
	/* Initialisation du tubes de synchronisation */
	/* Initialize master to client pipe */
	dp.lclnt[i] = malloc(sizeof(int) * 2);
	if (pipe(dp.lclnt[i]) < 0) {
		perror("Impossible to create pipe\n");
		exit(1);
	}
	P("Initialization %d\n", i);
	write(0, ".", 1);
	return df;
}

int initialise(int clnt)
{

	/* Initialisation des donnees publiques */
	/* Initialize private data fields */
	printf("Init\n");
	dp.nclnt = clnt;
	dp.lclnt = malloc(sizeof(int *) * clnt);
	dp.lthreads = malloc(sizeof(pthread_t) * clnt);

	/* initialisation de la communication avec le maitre */
	/* Initialize client to master pipe */
	if (pipe(dp.maitre) < 0) {
		perror("Master pipe creation error\n");
		exit(1);
	}
	printf("%s initialization\n", eType);
	load();
	initTest();

	return 0;
}

void cleanClient(struct donneesFils *df)
{
	int i;
	i = df->dpr->whoami;
	free(dp.lclnt[i]);
	free(df->dpr);
	free(df);
}

void clean()
{
	free(dp.lthreads);
	free(dp.lclnt);
}

int loadProcess()
{
	int i;
	struct donneesFils *df;
	for (i = 0; i < dp.nclnt; i++) {
		df = initClientFork(i);
		if (!fork()) {
			P("Running slave num: %d\n", df->dpr->whoami);
			write(0, ".", 1);
			esclave((void *)df);
			cleanClient(df);
			exit(0);
		}
	}
	return 0;
}

void lockWholeFile(struct flock *request)
{
	request->l_whence = SEEK_SET;
	request->l_start = 0;
	/* Lock de l'ensemble du fichier */
	/* Lock the whole file */
	request->l_len = 0;
}

void selectTest(int n, struct s_test *test)
{

	test->test = testSuiv(n);
	test->resAtt = resAttSuiv(n);
	test->nom = nomTestSuiv(n);
	test->type = lockSuiv(n);
}

/* Final test report */
void rapport(clnt)
{
	int i;
	int totalClients;
	totalClients = clnt * (maxClients + 1);
	printf
	    ("\n%s number : %d - Remote clients: %d local client 1 - Total client %d - Total concurent tests: %d\n",
	     eType, clnt, maxClients, maxClients + 1, totalClients);
	printf("%s number running test successfully :\n", eType);
	for (i = 0; i < MAXTEST; i++)
		printf("%d %s of %d successfully ran test : %s\n",
		       TOTAL_RESULTAT_OK[i], eType, totalClients,
		       LISTE_NOMS_TESTS[i]);

}

int serverSendLocal()
{
	int i;
	/* Synchronisation des processus esclaves. */
	/* Synchronize slave processes */

	/* On configure les esclaves pour le test */
	/* Configure slaves for test */

	for (i = 0; i < dp.nclnt; i++)
		write(dp.lclnt[i][1], message, M_SIZE);
	return 0;

}

void serverSendNet()
{
	writeToAllClients(message);
}

int serverReceiveNet()
{
	int i, c;
	for (c = 0; c < maxClients; c++) {
		for (i = 0; i < dp.nclnt; i++) {
			serverReceiveClient(c);
		}
	}
	return 0;
}

int serverReceiveLocal()
{
	int i;
	for (i = 0; i < dp.nclnt; i++)
		read(maitreLecteur, message, M_SIZE);
	return 0;
}

int clientReceiveLocal()
{
	read(esclaveLecteur, message, M_SIZE);
	return 0;
}

int clientSend()
{
	write(esclaveEcrivain, message, M_SIZE);
	return 0;
}

int serverSend()
{
	serverSendNet();
	serverSendLocal();
	return 0;
}

int serverReceive()
{
	serverReceiveNet();
	serverReceiveLocal();
	return 0;
}

/* binary structure <-> ASCII functions used to ensure data will be correctly used over
 * the network, especially when multiples clients do not use the same hardware architecture.
 */
int serialiseTLock(struct s_test *tLock)
{
	memset(message, 0, M_SIZE);
	sprintf(message, "T:%d:%d:%d::", tLock->test, tLock->type,
		tLock->resAtt);
	return 0;
}

void unSerialiseTLock(struct s_test *tLock)
{
	sscanf(message, "T:%d:%d:%d::", &(tLock->test), &(tLock->type),
	       &(tLock->resAtt));
	memset(message, 0, M_SIZE);

}

void serialiseFLock(struct flock *request)
{
	int len, pid, start;
	memset(message, 0, M_SIZE);
	len = (int)request->l_len;
	pid = (int)request->l_pid;
	start = (int)request->l_start;
	/* Beware to length of integer conversions ... */
	sprintf(message, "L:%hd:%hd:%d:%d:%d::",
		request->l_type, request->l_whence, start, len, pid);
}

void serialiseResult(int resultat)
{
	memset(message, 0, M_SIZE);
	sprintf(message, "R:%d::", resultat);

}

void unSerialiseResult(int *resultat)
{
	sscanf(message, "R:%d::", resultat);
}

void unSerialiseFLock(struct flock *request)
{
	int len, pid, start;
	sscanf(message, "L:%hd:%hd:%d:%d:%d::",
	       &(request->l_type), &(request->l_whence), &start, &len, &pid);
	request->l_start = (off_t) start;
	request->l_len = (off_t) len;
	request->l_pid = (pid_t) pid;
}

int serverSendLockClient(struct flock *request, int client)
{
	serialiseFLock(request);
	return serverSendClient(client);
}

int serverSendLockLocal(struct flock *request, int esclave)
{
	serialiseFLock(request);
	return write(dp.lclnt[esclave][1], message, M_SIZE);
}

int getLockSection(struct flock *request)
{
	memset(message, 0, M_SIZE);
	clientReceiveLocal();
	unSerialiseFLock(request);
	return 0;
}

int sendLockTest(struct s_test *tLock)
{
	serialiseTLock(tLock);
	serverSend();
	return 0;
}

int getLockTest(struct s_test *tLock)
{
	clientReceiveLocal();
	unSerialiseTLock(tLock);
	return 0;
}

int sendResult(int resultat)
{
	serialiseResult(resultat);
	clientSend();
	return 0;
}

int getResults(int ntest)
{
	int i, c;
	int resultat = 0;
	/* On comptabilise les resultats distants */
	/* Add remote test results */
	for (c = 0; c < maxClients; c++) {
		for (i = 0; i < dp.nclnt; i++) {
			serverReceiveClient(c);
			unSerialiseResult(&resultat);
			compteur(resultat, ntest);

		}
	}
	/* On comptabilise les resultats locaux */
	/* Add local test results */
	for (i = 0; i < dp.nclnt; i++) {
		read(maitreLecteur, message, M_SIZE);
		unSerialiseResult(&resultat);
		compteur(resultat, ntest);
	}

	return 0;
}

/* Usefull debug macro */
#ifdef DEBUG
#define P(a,b)  memset(dbg,0,16);sprintf(dbg,a,b);write(0,dbg,16);
#endif

/* Le maitre de l'application client n'est qu'un repetiteur d'information.
 * Il retransmet l'information qu'il reçoit du maitre du client a ses esclaves
 */
/* In the case of a network use, the master of the client application si only
 * a 'repeater' of information. It resends server-master instructions to its own slaves.
 */

void maitreClient()
{
	fd_set fdread;
	struct timeval tv;
	int n, i, r, m, start;
#ifdef DEBUG
	char dbg[16];
#endif
	struct flock lock;
	int t;

	maitreLecteur = dp.maitre[0];
	FD_ZERO(&fdread);
	tv.tv_sec = 50;
	tv.tv_usec = 0;
	n = fdServeur > maitreLecteur ? fdServeur : maitreLecteur;
	printf("Maitre CLient - fdServeur=%d\n", fdServeur);
	while (1) {
		/* On ajoute les descripteurs esclave et serveur */
		/* Add slave and server pipe file descriptors */
		FD_ZERO(&fdread);
		FD_SET(fdServeur, &fdread);
		FD_SET(maitreLecteur, &fdread);
		r = select(n + 1, &fdread, NULL, NULL, &tv);
		if (r < 0) {
			perror("select:\n");
			continue;
		}
		if (r == 0) {
			exit(0);
		}

		if (FD_ISSET(fdServeur, &fdread)) {
			/* On vient de recevoir une information du serveur
			 * On la retransmet aux esclaves
			 */
			/* We just have received information from the server.
			 * We repeat it to slaves.
			 */
			i = readFromServer(message);
			t = message[0];
			switch (t) {
			case 'L':
				/* Instruction : Lock. Dans ce cas il faut envoyer a chaque processus une section differente a locker */
				/* Lock instruction. We need to send a different section to lock to each process */

				unSerialiseFLock(&lock);
				start = lock.l_start;
				for (i = 0; i < dp.nclnt; i++) {
					lock.l_start = start + i;
					serialiseFLock(&lock);
					write(dp.lclnt[i][1], message, M_SIZE);
				}
				printf("\n");
				continue;
			case 'T':
				/* Instruction: Test. Il s'agit d'une trame annoncant un nouveau test : on verifie qu'il ne s'agit pas d'une
				 * demande de FIN des tests
				 */
				/* Test instruction. Ensure server is not sending the FIN(ish) instruction to end tests */

				/* A re-ecrire un peu mieux des que possible */
				/* To be rewritten asap */
				m = atoi(&(message[2]));
				if (m == FIN)
					break;
				if (m == CLEAN)
					printf("\n");

				serverSendLocal();
				continue;
			}
			break;
		} else {
			/* Dans le cas inverse, on lis les esclaves et on remonte l'information au serveur
			 */
			/* Else, we read information from slaves and repeat them to the server */
			for (i = 0; i < dp.nclnt; i++) {
				r = read(maitreLecteur, message, M_SIZE);
				r = write(fdServeur, message, M_SIZE);
				if (r < 0)
					perror("write : ");

			}
			continue;
		}
	}

	/* On vient de recevoir la trame FIN de programme */
	/* Receive the FIN(ish) instruction */

	/* On la communique a tous les esclaves */
	/* Repeat it to the slaves */
	printf("Fin du programme en cours...\n");
	serverSendLocal();

	/* Et c'est fini! */
	/* Ok, we can quit */
	printf("Bye :)\n");

}

void maitre()
{
	int i, n, bl;
	int clnt;
	char tmp[MAXLEN], *buf;
#ifdef DEBUG
	char dbg[16];
#endif
	struct flock request;
	struct s_test tLock;
	enum etat_t etat;
	int offset;
	/* A test sentence written in the file */
	char phraseTest[] =
	    "Ceci est une phrase test ecrite par le maitre dans le fichier";
	bl = -1;
	clnt = dp.nclnt;
	maitreLecteur = dp.maitre[0];
	etat = SELECT;
	/* On commence par le premier test. C'est original ;) */
	/* Start with the first test ;) */
	n = 0;
	printf("\n--------------------------------------\n");
	while (1) {
		switch (etat) {
		case SELECT:
			/* Selection du test a effectuer */
			/* Select the test to perform   */
			printf("\n");
			E("Maitre: SELECT");
			selectTest(n, &tLock);
			etat = tLock.type;
			bl = 0;
			if (n < MAXTEST) {
				memset(tmp, 0, MAXLEN);
				sprintf(tmp, "TEST : TRY TO %s:",
					LISTE_NOMS_TESTS[n]);
				write(0, tmp, strlen(tmp));
			} else
				etat = FIN;
			P("etat=%d\n", etat);
			n += 1;
			continue;

		case RDONLY:
		case WRONLY:

		case READLOCK:
			P("Read lock :%d\n", etat);
			request.l_type = F_RDLCK;
			etat = LOCK;
			continue;

		case WRITELOCK:
			P("Write lock :%d\n", etat);
			request.l_type = F_WRLCK;
			etat = LOCK;
			continue;

		case LOCK:
			/* On applique le lock que l'on veut */
			/* Apply the wanted lock */
			E("Maitre: LOCK");
			write(dp.fd, phraseTest, strlen(phraseTest));
			lockWholeFile(&request);
			if (fcntl(dp.fd, F_SETLK, &request) < 0) {
				perror("Master: can't set lock\n");
				perror("Echec\n");
				exit(0);
			}
			E("Maitre");
			etat = SYNC;
			continue;

		case BYTELOCK_READ:
			bl = 1;
			request.l_type = F_RDLCK;
			etat = SYNC;
			continue;

		case BYTELOCK_WRITE:
			bl = 1;
			request.l_type = F_WRLCK;
			etat = SYNC;
			continue;

		case BYTELOCK:
			/*
			 * L'idee est de faire locker l'ensemble du fichier octet par octet par un ensemble de sous processus
			 * Il nous faut donc
			 * -creer un fichier ayant autant d'octet que le nombre de processus passe en parametres
			 * -passer les sections a locker a chacun des esclaves
			 * -verifier que les locks sont bien appliques
			 *
			 */

			/* The main idea is to lock all the bytes in a file. Each slave process locks one byte.
			 *
			 * We need :
			 * - To create a file of a length equal to the total number of slave processes
			 * - send the exact section to lock to each slave
			 * - ensure locks have been correctly set
			 */

			/* On cree une chaine de caracteres a enregistrer dans le fichier qui contienne exactement un octet par
			 * processus.
			 */
			/* Create a string to record in the test file. Length is exactly the number of sub process */
			P("Maitre: BYTELOCK: %d\n", etat);
			buf = malloc(clnt * (maxClients + 1));
			memset(buf, '*', clnt);
			write(dp.fd, buf, clnt);
			free(buf);

			/* Chaque processus esclave reecrit son champs a locker. */
			/* Each slave process re-writes its own field to lock */
			request.l_whence = SEEK_SET;
			request.l_start = 0;
			request.l_len = 1;

			/* On commence par les envois reseau */
			/* Start to send sections to lock to remote process (network clients) */

			for (i = 0; i < maxClients; i++) {
				/* On renseigne la structure avec le lock qui va bien */
				/* Set the correct byte to lock */
				offset = (i + 1) * clnt;
				request.l_start = (off_t) offset;
				serverSendLockClient(&request, i);
			}

			/* Puis les envois locaux */
			/* Now send sections to local processes */
			for (i = 0; i < clnt; i++) {
				request.l_start = i;
				serverSendLockLocal(&request, i);
			}
			etat = RESULTAT;
			continue;

		case SYNC:
			sendLockTest(&tLock);
			if (bl) {
				etat = BYTELOCK;
				continue;
			}

			if (n < MAXTEST + 1)
				etat = RESULTAT;
			else
				etat = FIN;
			continue;

		case RESULTAT:
			/* On lit les resultats un par un */
			/* Read results by one */
			getResults(n - 1);
			if (bl)
				validationResultats(n - 1);
			etat = CLEAN;
			continue;

		case CLEAN:
			/* On demande aux clients d'arreter le test */
			/* Ask the clients to stop testing ... */
			tLock.test = CLEAN;
			serialiseTLock(&tLock);
			serverSend();
			/* ... et on attend un accuse de reception avant de fermer */
			/* ... and wait for an ack before closing */
			serverReceive();
			/* On ignore les resultats, ce n'est qu'un accuse de reception */
			/* Ignore message content : that is only an ack */

			/* close and open file */
			close(dp.fd);
			initTest(dp);
			etat = SELECT;
			continue;
		case FIN:
			tLock.test = FIN;
			serialiseTLock(&tLock);
			serverSend();
			sleep(2);
			break;

			printf("(end)\n");
			exit(0);

		}		/* switch */
		break;
	}			/* while */

	rapport(clnt);
}

/* Slave process/thread */

void *esclave(void *donnees)
{
	struct donneesFils *df;
	int i, a, resultat, ftest;
	struct s_test tLock;
	struct flock request;
	char tmp[16];
#ifdef DEBUG
	char dbg[16];
#endif
	char *phraseTest = "L'ecriture a reussi";
	int len;
	enum etat_t etat;

	resultat = -1;
	ftest = -1;
	len = strlen(phraseTest);
	df = (struct donneesFils *)donnees;
	i = df->dpr->whoami;
	P("Esclave n=%d\n", i);
	esclaveLecteur = dp.lclnt[i][0];
	esclaveEcrivain = dp.maitre[1];
	etat = SYNC;
	errno = 0;
	memset(tmp, 0, 16);
	while (1) {
		switch (etat) {
		case SELECT:
		case SYNC:
			getLockTest(&tLock);
			etat = tLock.test;
			P("Esclave Etat=%d\n", etat);

			continue;
		case RDONLY:
			/* On tente de lire un fichier */
			/* Try to read a file */
			P("TEST READ ONLY %d\n", RDONLY);
			if ((ftest = open(dp.fname, O_RDONLY | O_NONBLOCK)) < 0) {
				resultat = ECHEC;
				etat = RESULTAT;
				if (dp.verbose)
					perror("Open:");
				continue;
			}
			P("fd=%d\n", ftest);
			a = read(ftest, tmp, 16);
			if (a < 16)
				resultat = ECHEC;
			else
				resultat = SUCCES;
			etat = RESULTAT;
			continue;

		case WRONLY:
			/* On tente d'ecrire un fichier */
			/* Try to write a file */
			P("TEST WRITE ONLY %d\n", WRONLY);
			if ((ftest = open(dp.fname, O_WRONLY | O_NONBLOCK)) < 0) {
				resultat = ECHEC;
				etat = RESULTAT;
				if (dp.verbose)
					perror("Open read only:");
				continue;
			}
			P("fd=%d\n", ftest);
			if (write(ftest, phraseTest, len) < len)
				resultat = ECHEC;
			else
				resultat = SUCCES;
			etat = RESULTAT;
			continue;

		case LOCK:

		case READLOCK:
			/* On essaie de lire une section lockee en lecture sur le fichier */
			/* Try to read a read-locked section */
			P("READ LOCK %d\n", F_RDLCK);
			if ((ftest = open(dp.fname, O_RDONLY | O_NONBLOCK)) < 0) {
				resultat = ECHEC;
				etat = RESULTAT;
				if (dp.verbose)
					perror("Open read-write:");
				continue;
			}

			P("fd=%d\n", ftest);
			/* Lock de l'ensemble du fichier */
			/* Lock the whole file */
			request.l_type = F_RDLCK;
			request.l_whence = SEEK_SET;
			request.l_start = 0;
			request.l_len = 0;

			if (fcntl(ftest, F_SETLK, &request) < 0) {
				if (dp.verbose || errno != EAGAIN)
					perror("RDONLY: fcntl");
				resultat = ECHEC;
			} else
				resultat = SUCCES;
			etat = RESULTAT;
			continue;

		case WRITELOCK:
			/* On essaie d'ecrire le fichier */
			/* Try to write a file */
			P("WRITE LOCK %d\n", F_WRLCK);
			if ((ftest = open(dp.fname, O_WRONLY | O_NONBLOCK)) < 0) {
				resultat = ECHEC;
				etat = RESULTAT;
				if (dp.verbose)
					perror("\nOpen:");
				continue;
			}
			/* Lock de l'ensemble du fichier */
			/* Lock the whole file */
			P("fd=%d\n", ftest);
			request.l_type = F_WRLCK;
			request.l_whence = SEEK_SET;
			request.l_start = 0;
			request.l_len = 0;

			if (fcntl(ftest, F_SETLK, &request) < 0) {
				if (dp.verbose || errno != EAGAIN)
					perror("\nWRONLY: fcntl");
				resultat = ECHEC;
			} else
				resultat = SUCCES;
			etat = RESULTAT;
			continue;

		case BYTELOCK_READ:
			P("BYTE LOCK READ: %d\n", etat);
			etat = BYTELOCK;
			continue;

		case BYTELOCK_WRITE:
			P("BYTE LOCK WRITE: %d\n", etat);
			etat = BYTELOCK;
			continue;

		case BYTELOCK:
			/* On se met en attente de la section a locker. L'ensemble de la structure est
			 * donnee par le maitre et transmise par le tube.
			 */
			/* Wait for the exact section to lock. The whole file is sent by the master */

			P("BYTE LOCK %d\n", etat);
			getLockSection(&request);
			if ((ftest = open(dp.fname, O_RDWR | O_NONBLOCK)) < 0) {
				resultat = ECHEC;
				etat = RESULTAT;
				if (dp.verbose)
					perror("\nOpen:");
				continue;
			}

			if (fcntl(ftest, F_SETLK, &request) < 0) {
				if (dp.verbose || errno != EAGAIN)
					perror("\nBTLOCK: fcntl");
				resultat = ECHEC;
				etat = RESULTAT;
				continue;
			}
			/* On change le caractere a la place donnee pour verification */
			/* Change the character at the given position for an easier verification */
			a = lseek(ftest, request.l_start, SEEK_SET);
			write(ftest, "=", 1);
			resultat = SUCCES;
			etat = RESULTAT;
			continue;

		case RESULTAT:
			if (resultat == SUCCES)
				write(0, "=", 1);
			else
				write(0, "x", 1);
			P("RESULTAT: %d\n", resultat);
			sendResult(resultat);
			etat = SYNC;
			continue;

		case CLEAN:
			close(ftest);
			/* Send CLEAN Ack */
			sendResult(resultat);
			etat = SYNC;
			continue;

		case FIN:
			E("(End)\n");
			terminer(0);
			printf("Unknown command\n");
			terminer(1);

		}
	}

}

char *nextArg(int argc, char **argv, int *i)
{
	if (((*i) + 1) < argc) {
		(*i) += 1;
		return argv[(*i)];
	}
	return NULL;
}

int usage()
{
	printf("locktest -n <number of process> -f <test file> [-T]\n");
	printf("Number of child process must be higher than 1\n");
	printf("\n");
	printf("Send bugs to vincent.roqueta@ext.bull.net\n");
	exit(0);
	return 0;
}

int main(int argc, char **argv)
{
	int i, nThread = 0;
	char *tmp;
	int type = 0;
	int clients;
	type = PROCESS;
	dp.fname = NULL;
	dp.verbose = 0;
	int serveur = 1;
	char *host;

	host = NULL;
	clients = 0;

	for (i = 1; i < argc; i++) {

		if (!strcmp("-n", argv[i])) {
			if (!(tmp = nextArg(argc, argv, &i)))
				usage();
			nThread = atoi(tmp);
			continue;
		}

		if (!strcmp("-f", argv[i])) {
			if (!(dp.fname = nextArg(argc, argv, &i)))
				usage();
			continue;
		}
		if (!strcmp("-v", argv[i])) {
			dp.verbose = TRUE;
			continue;
		}
		if (!strcmp("-c", argv[i])) {
			if (!(clients = atoi(nextArg(argc, argv, &i))))
				usage();
			continue;
		}

		if (!strcmp("--server", argv[i])) {
			if (!(host = nextArg(argc, argv, &i)))
				usage();
			serveur = 0;
			continue;
		}
		/* Option inconnue */
		printf("Ignoring unknown option: %s\n", argv[i]);
	}

	if (serveur) {
		if (!(dp.fname && nThread))
			usage();
		if (clients > 0) {
			configureServeur(clients);
			setupClients(type, dp.fname, nThread);
		}
	} else {
		configureClient(host);
		dp.fname = malloc(512);
		memset(dp.fname, 0, 512);
		getConfiguration(&type, dp.fname, &nThread);
	}

	if (dp.verbose)
		printf("By process.\n");
	load = loadProcess;
	eType = "process";
	terminer = terminerProcess;
	LISTE_RESULTATS = LISTE_RESULTATS_PROCESS;
	initialise(nThread);
	if (serveur) {
		maitre();

	} else {
		maitreClient();
		free(dp.fname);
	}
	clean();

	return 0;
}
