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
extern int fdServer;

char message[M_SIZE];
int slaveReader;
int masterReader;
int slaveWriter;

/* Which lock will be applied by the master process on test startup */
int LIST_LOCKS[] = { READLOCK, WRITELOCK,
	READLOCK, WRITELOCK,
	READLOCK, WRITELOCK,
	READLOCK, WRITELOCK,
	BYTELOCK_READ, BYTELOCK_WRITE
};

/* The operations the slave processes will try to perform */
int LIST_TESTS[] = { WRONLY, WRONLY,
	RDONLY, RDONLY,
	READLOCK, WRITELOCK,
	WRITELOCK, READLOCK,
	BYTELOCK_READ, BYTELOCK_WRITE
};

/* List of test names */
char *LIST_NAMES_TESTS[] = { "WRITE ON A READ  LOCK",
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

/* List of expected test results, when slaves are processes */
int LIST_RESULTS_PROCESS[] = { SUCCES, SUCCES,
	SUCCES, SUCCES,
	SUCCES, ECHEC,
	ECHEC, ECHEC,
	SUCCES, SUCCES
};

/* List of expected test results, when slaves are threads */
int LIST_RESULTS_THREADS[] = { SUCCES, SUCCES,
	SUCCES, SUCCES,
	SUCCES, SUCCES,
	SUCCES, SUCCES,
	ECHEC, ECHEC
};

int *LIST_RESULTS = NULL;
char *eType = NULL;

int TOTAL_RESULT_OK[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void *slave(void *data);
int (*finish) (int a);

int finishProcess(int a)
{
	exit(a);
}

int (*load) (void);

struct dataPub dp;

/* Functions to access tests/tests names/tests results*/
int testSuiv(int n)
{
	return LIST_TESTS[n];
}

int resAttSuiv(int n)
{
	return LIST_RESULTS[n];
}

char *nomTestSuiv(int n)
{
	return LIST_NAMES_TESTS[n];
}

int lockSuiv(int n)
{
	return LIST_LOCKS[n];
}

/* Verify the test result is the expected one */
int matchResult(int r, int n)
{

	P("r=%d\n", r);
	if (r == LIST_RESULTS[n])
		return 1;
	else
		return 0;
}

/* Increments the number of process which have successfully passed the test */
void counter(int r, int n)
{
	TOTAL_RESULT_OK[n] += matchResult(r, n);
}

/* Special case for test 'lock file byte byte by byte'.
 * We ensure each byte is correctly locked.
 */
void validationResults(int n)
{
	int i, u, l, fsize;
	struct flock request;

	fsize = dp.nclnt * (maxClients + 1);
	TOTAL_RESULT_OK[n] = 0;
	l = FALSE;
	u = TRUE;

	/* If the expected operation result is a success, we will have to increase the number of correct results */
	if (LIST_RESULTS[n]) {
		l = TRUE;
		u = FALSE;
	}

	for (i = 0; i < fsize; i++) {
		request.l_type = F_WRLCK;
		request.l_whence = SEEK_SET;
		request.l_start = i;
		request.l_len = 1;
		fcntl(dp.fd, F_GETLK, &request);
		/* Ensure the lock is correctly set */
		if (request.l_type != F_UNLCK)
			TOTAL_RESULT_OK[n] += l;
		else
			TOTAL_RESULT_OK[n] += u;
	}
}

int initTest(void)
{

	P("Master opens %s\n", dp.fname);
	dp.fd = open(dp.fname, OPENFLAGS, MANDMODES);
	if (dp.fd < 0) {
		perror("lock test : can't open test file :");
		finish(1);
	}
	P("fd=%d\n", dp.fd);
	return 0;
}

struct dataChild *initClientFork(int i)
{
	struct dataPriv *dpr;
	struct dataChild *df;

	/* Initialize private data fields */
	dpr = malloc(sizeof(struct dataPriv));
	df = malloc(sizeof(struct dataChild));
	dpr->whoami = i;
	df->dp = &dp;
	df->dpr = dpr;
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

int initialize(int clnt)
{

	/* Initialize private data fields */
	printf("Init\n");
	dp.nclnt = clnt;
	dp.lclnt = malloc(sizeof(int *) * clnt);
	dp.lthreads = malloc(sizeof(pthread_t) * clnt);

	/* Initialize client to master pipe */
	if (pipe(dp.master) < 0) {
		perror("Master pipe creation error\n");
		exit(1);
	}
	printf("%s initialization\n", eType);
	load();
	initTest();

	return 0;
}

void cleanClient(struct dataChild *df)
{
	int i;
	i = df->dpr->whoami;
	free(dp.lclnt[i]);
	free(df->dpr);
	free(df);
}

void clean(void)
{
	free(dp.lthreads);
	free(dp.lclnt);
}

int loadProcess(void)
{
	int i;
	struct dataChild *df;
	for (i = 0; i < dp.nclnt; i++) {
		df = initClientFork(i);
		if (!fork()) {
			P("Running slave num: %d\n", df->dpr->whoami);
			write(0, ".", 1);
			slave((void *)df);
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
int report(int clnt)
{
	int rc = 0;
	int i;
	int totalClients;
	totalClients = clnt * (maxClients + 1);
	printf
	    ("\n%s number : %d - Remote clients: %d local client 1 - Total client %d - Total concurent tests: %d\n",
	     eType, clnt, maxClients, maxClients + 1, totalClients);
	printf("%s number running test successfully :\n", eType);
	for (i = 0; i < MAXTEST; i++) {
		if (TOTAL_RESULT_OK[i] != totalClients)
			rc = 1;

		printf("%d %s of %d successfully ran test : %s\n",
		       TOTAL_RESULT_OK[i], eType, totalClients,
		       LIST_NAMES_TESTS[i]);
    }
    return rc;
}

int serverSendLocal(void)
{
	int i;
	/* Synchronize slave processes */
	/* Configure slaves for test */

	for (i = 0; i < dp.nclnt; i++)
		write(dp.lclnt[i][1], message, M_SIZE);
	return 0;

}

void serverSendNet(void)
{
	writeToAllClients(message);
}

int serverReceiveNet(void)
{
	int i, c;
	for (c = 0; c < maxClients; c++) {
		for (i = 0; i < dp.nclnt; i++) {
			serverReceiveClient(c);
		}
	}
	return 0;
}

int serverReceiveLocal(void)
{
	int i;
	for (i = 0; i < dp.nclnt; i++)
		read(masterReader, message, M_SIZE);
	return 0;
}

int clientReceiveLocal(void)
{
	read(slaveReader, message, M_SIZE);
	return 0;
}

int clientSend(void)
{
	write(slaveWriter, message, M_SIZE);
	return 0;
}

int serverSend(void)
{
	serverSendNet();
	serverSendLocal();
	return 0;
}

int serverReceive(void)
{
	serverReceiveNet();
	serverReceiveLocal();
	return 0;
}

/* binary structure <-> ASCII functions used to ensure data will be correctly used over
 * the network, especially when multiples clients do not use the same hardware architecture.
 */
int serializeTLock(struct s_test *tLock)
{
	memset(message, 0, M_SIZE);
	sprintf(message, "T:%d:%d:%d::", tLock->test, tLock->type,
		tLock->resAtt);
	return 0;
}

void unSerializeTLock(struct s_test *tLock)
{
	sscanf(message, "T:%d:%d:%d::", &(tLock->test), &(tLock->type),
	       &(tLock->resAtt));
	memset(message, 0, M_SIZE);

}

void serializeFLock(struct flock *request)
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

void serializeResult(int result)
{
	memset(message, 0, M_SIZE);
	sprintf(message, "R:%d::", result);

}

void unSerializeResult(int *result)
{
	sscanf(message, "R:%d::", result);
}

void unSerializeFLock(struct flock *request)
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
	serializeFLock(request);
	return serverSendClient(client);
}

int serverSendLockLocal(struct flock *request, int slave)
{
	serializeFLock(request);
	return write(dp.lclnt[slave][1], message, M_SIZE);
}

int getLockSection(struct flock *request)
{
	memset(message, 0, M_SIZE);
	clientReceiveLocal();
	unSerializeFLock(request);
	return 0;
}

int sendLockTest(struct s_test *tLock)
{
	serializeTLock(tLock);
	serverSend();
	return 0;
}

int getLockTest(struct s_test *tLock)
{
	clientReceiveLocal();
	unSerializeTLock(tLock);
	return 0;
}

int sendResult(int result)
{
	serializeResult(result);
	clientSend();
	return 0;
}

int getResults(int ntest)
{
	int i, c;
	int result = 0;
	/* Add remote test results */
	for (c = 0; c < maxClients; c++) {
		for (i = 0; i < dp.nclnt; i++) {
			serverReceiveClient(c);
			unSerializeResult(&result);
			counter(result, ntest);

		}
	}
	/* Add local test results */
	for (i = 0; i < dp.nclnt; i++) {
		read(masterReader, message, M_SIZE);
		unSerializeResult(&result);
		counter(result, ntest);
	}

	return 0;
}

#ifdef DEBUG
#define P(a,b)  memset(dbg,0,16);sprintf(dbg,a,b);write(0,dbg,16);
#endif

/* In the case of a network use, the master of the client application si only
 * a 'repeater' of information. It resends server-master instructions to its own slaves.
 */
void masterClient(void)
{
	fd_set fdread;
	struct timeval tv;
	int n, i, r, m, start;
#ifdef DEBUG
	char dbg[16];
#endif
	struct flock lock;
	int t;

	masterReader = dp.master[0];
	FD_ZERO(&fdread);
	tv.tv_sec = 50;
	tv.tv_usec = 0;
	n = fdServer > masterReader ? fdServer : masterReader;
	printf("Master Client - fdServer=%d\n", fdServer);
	while (1) {
		/* Add slave and server pipe file descriptors */
		FD_ZERO(&fdread);
		FD_SET(fdServer, &fdread);
		FD_SET(masterReader, &fdread);
		r = select(n + 1, &fdread, NULL, NULL, &tv);
		if (r < 0) {
			perror("select:\n");
			continue;
		}
		if (r == 0) {
			exit(0);
		}

		if (FD_ISSET(fdServer, &fdread)) {
			/* We just have received information from the server.
			 * We repeat it to slaves.
			 */
			i = readFromServer(message);
			t = message[0];
			switch (t) {
			case 'L':
				/* Lock instruction. We need to send a different section to lock to each process */
				unSerializeFLock(&lock);
				start = lock.l_start;
				for (i = 0; i < dp.nclnt; i++) {
					lock.l_start = start + i;
					serializeFLock(&lock);
					write(dp.lclnt[i][1], message, M_SIZE);
				}
				printf("\n");
				continue;
			case 'T':
				/* Test instruction. Ensure server is not sending the END(ish) instruction to end tests */
				/* To be rewritten asap */
				m = atoi(&(message[2]));
				if (m == END)
					break;
				if (m == CLEAN)
					printf("\n");

				serverSendLocal();
				continue;
			}
			break;
		} else {
			/* Else, we read information from slaves and repeat them to the server */
			for (i = 0; i < dp.nclnt; i++) {
				r = read(masterReader, message, M_SIZE);
				r = write(fdServer, message, M_SIZE);
				if (r < 0)
					perror("write : ");

			}
			continue;
		}
	}

	/* Receive the END(ish) instruction */

	/* Repeat it to the slaves */
	printf("Exitting...\n");
	serverSendLocal();

	/* Ok, we can quit */
	printf("Bye :)\n");

}

int master(void)
{
	int i, n, bl;
	int clnt;
	char tmp[MAXLEN], *buf;
#ifdef DEBUG
	char dbg[16];
#endif
	struct flock request;
	struct s_test tLock;
	enum state_t state;
	int offset;
	/* A test sentence written in the file */
	char phraseTest[] =
	    "Ceci est une phrase test ecrite par le maitre dans le fichier";
	bl = -1;
	clnt = dp.nclnt;
	masterReader = dp.master[0];
	state = SELECT;
	/* Start with the first test ;) */
	n = 0;
	printf("\n--------------------------------------\n");
	while (1) {
		switch (state) {
		case SELECT:
			/* Select the test to perform   */
			printf("\n");
			E("Master: SELECT");
			selectTest(n, &tLock);
			state = tLock.type;
			bl = 0;
			if (n < MAXTEST) {
				memset(tmp, 0, MAXLEN);
				sprintf(tmp, "TEST : TRY TO %s:",
					LIST_NAMES_TESTS[n]);
				write(0, tmp, strlen(tmp));
			} else
				state = END;
			P("state=%d\n", state);
			n += 1;
			continue;

		case RDONLY:
		case WRONLY:

		case READLOCK:
			P("Read lock :%d\n", state);
			request.l_type = F_RDLCK;
			state = LOCK;
			continue;

		case WRITELOCK:
			P("Write lock :%d\n", state);
			request.l_type = F_WRLCK;
			state = LOCK;
			continue;

		case LOCK:
			/* Apply the wanted lock */
			E("Master: LOCK");
			write(dp.fd, phraseTest, strlen(phraseTest));
			lockWholeFile(&request);
			if (fcntl(dp.fd, F_SETLK, &request) < 0) {
				perror("Master: can't set lock\n");
				perror("Echec\n");
				exit(0);
			}
			E("Master");
			state = SYNC;
			continue;

		case BYTELOCK_READ:
			bl = 1;
			request.l_type = F_RDLCK;
			state = SYNC;
			continue;

		case BYTELOCK_WRITE:
			bl = 1;
			request.l_type = F_WRLCK;
			state = SYNC;
			continue;

		case BYTELOCK:
			/* The main idea is to lock all the bytes in a file. Each slave process locks one byte.
			 *
			 * We need :
			 * - To create a file of a length equal to the total number of slave processes
			 * - send the exact section to lock to each slave
			 * - ensure locks have been correctly set
			 */

			/* Create a string to record in the test file. Length is exactly the number of sub process */
			P("Master: BYTELOCK: %d\n", state);
			buf = malloc(clnt * (maxClients + 1));
			memset(buf, '*', clnt);
			write(dp.fd, buf, clnt);
			free(buf);

			/* Each slave process re-writes its own field to lock */
			request.l_whence = SEEK_SET;
			request.l_start = 0;
			request.l_len = 1;

			/* Start to send sections to lock to remote process (network clients) */
			for (i = 0; i < maxClients; i++) {
				/* Set the correct byte to lock */
				offset = (i + 1) * clnt;
				request.l_start = (off_t) offset;
				serverSendLockClient(&request, i);
			}

			/* Now send sections to local processes */
			for (i = 0; i < clnt; i++) {
				request.l_start = i;
				serverSendLockLocal(&request, i);
			}
			state = RESULT;
			continue;

		case SYNC:
			sendLockTest(&tLock);
			if (bl) {
				state = BYTELOCK;
				continue;
			}

			if (n < MAXTEST + 1)
				state = RESULT;
			else
				state = END;
			continue;

		case RESULT:
			/* Read results by one */
			getResults(n - 1);
			if (bl)
				validationResults(n - 1);
			state = CLEAN;
			continue;

		case CLEAN:
			/* Ask the clients to stop testing ... */
			tLock.test = CLEAN;
			serializeTLock(&tLock);
			serverSend();
			/* ... and wait for an ack before closing */
			serverReceive();
			/* Ignore message content : that is only an ack */

			/* close and open file */
			close(dp.fd);
			initTest();
			state = SELECT;
			continue;
		case END:
			tLock.test = END;
			serializeTLock(&tLock);
			serverSend();
			sleep(2);
			break;

			printf("(end)\n");
			exit(0);

		}
		break;
	}

	return report(clnt);
}

/* Slave process/thread */

void *slave(void *data)
{
	struct dataChild *df;
	int i, a, result, ftest;
	struct s_test tLock;
	struct flock request;
	char tmp[16];
#ifdef DEBUG
	char dbg[16];
#endif
	char *phraseTest = "L'ecriture a reussi";
	int len;
	enum state_t state;

	result = -1;
	ftest = -1;
	len = strlen(phraseTest);
	df = (struct dataChild *)data;
	i = df->dpr->whoami;
	P("Slave n=%d\n", i);
	slaveReader = dp.lclnt[i][0];
	slaveWriter = dp.master[1];
	state = SYNC;
	errno = 0;
	memset(tmp, 0, 16);
	while (1) {
		switch (state) {
		case SELECT:
		case SYNC:
			getLockTest(&tLock);
			state = tLock.test;
			P("Slave State=%d\n", state);

			continue;
		case RDONLY:
			/* Try to read a file */
			P("TEST READ ONLY %d\n", RDONLY);
			if ((ftest = open(dp.fname, O_RDONLY | O_NONBLOCK)) < 0) {
				result = ECHEC;
				state = RESULT;
				if (dp.verbose)
					perror("Open:");
				continue;
			}
			P("fd=%d\n", ftest);
			a = read(ftest, tmp, 16);
			if (a < 16)
				result = ECHEC;
			else
				result = SUCCES;
			state = RESULT;
			continue;

		case WRONLY:
			/* Try to write a file */
			P("TEST WRITE ONLY %d\n", WRONLY);
			if ((ftest = open(dp.fname, O_WRONLY | O_NONBLOCK)) < 0) {
				result = ECHEC;
				state = RESULT;
				if (dp.verbose)
					perror("Open read only:");
				continue;
			}
			P("fd=%d\n", ftest);
			if (write(ftest, phraseTest, len) < len)
				result = ECHEC;
			else
				result = SUCCES;
			state = RESULT;
			continue;

		case LOCK:

		case READLOCK:
			/* Try to read a read-locked section */
			P("READ LOCK %d\n", F_RDLCK);
			if ((ftest = open(dp.fname, O_RDONLY | O_NONBLOCK)) < 0) {
				result = ECHEC;
				state = RESULT;
				if (dp.verbose)
					perror("Open read-write:");
				continue;
			}

			P("fd=%d\n", ftest);
			/* Lock the whole file */
			request.l_type = F_RDLCK;
			request.l_whence = SEEK_SET;
			request.l_start = 0;
			request.l_len = 0;

			if (fcntl(ftest, F_SETLK, &request) < 0) {
				if (dp.verbose || errno != EAGAIN)
					perror("RDONLY: fcntl");
				result = ECHEC;
			} else
				result = SUCCES;
			state = RESULT;
			continue;

		case WRITELOCK:
			/* Try to write a file */
			P("WRITE LOCK %d\n", F_WRLCK);
			if ((ftest = open(dp.fname, O_WRONLY | O_NONBLOCK)) < 0) {
				result = ECHEC;
				state = RESULT;
				if (dp.verbose)
					perror("\nOpen:");
				continue;
			}
			/* Lock the whole file */
			P("fd=%d\n", ftest);
			request.l_type = F_WRLCK;
			request.l_whence = SEEK_SET;
			request.l_start = 0;
			request.l_len = 0;

			if (fcntl(ftest, F_SETLK, &request) < 0) {
				if (dp.verbose || errno != EAGAIN)
					perror("\nWRONLY: fcntl");
				result = ECHEC;
			} else
				result = SUCCES;
			state = RESULT;
			continue;

		case BYTELOCK_READ:
			P("BYTE LOCK READ: %d\n", state);
			state = BYTELOCK;
			continue;

		case BYTELOCK_WRITE:
			P("BYTE LOCK WRITE: %d\n", state);
			state = BYTELOCK;
			continue;

		case BYTELOCK:
			/* Wait for the exact section to lock. The whole file is sent by the master */
			P("BYTE LOCK %d\n", state);
			getLockSection(&request);
			if ((ftest = open(dp.fname, O_RDWR | O_NONBLOCK)) < 0) {
				result = ECHEC;
				state = RESULT;
				if (dp.verbose)
					perror("\nOpen:");
				continue;
			}

			if (fcntl(ftest, F_SETLK, &request) < 0) {
				if (dp.verbose || errno != EAGAIN)
					perror("\nBTLOCK: fcntl");
				result = ECHEC;
				state = RESULT;
				continue;
			}
			/* Change the character at the given position for an easier verification */
			a = lseek(ftest, request.l_start, SEEK_SET);
			write(ftest, "=", 1);
			result = SUCCES;
			state = RESULT;
			continue;

		case RESULT:
			if (result == SUCCES)
				write(0, "=", 1);
			else
				write(0, "x", 1);
			P("RESULT: %d\n", result);
			sendResult(result);
			state = SYNC;
			continue;

		case CLEAN:
			close(ftest);
			/* Send CLEAN Ack */
			sendResult(result);
			state = SYNC;
			continue;

		case END:
			E("(End)\n");
			finish(0);
			printf("Unknown command\n");
			finish(1);

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

int usage(void)
{
	printf("locktest -n <number of process> -f <test file> [-T]\n");
	printf("Number of child process must be higher than 1\n");
	exit(0);
	return 0;
}

int main(int argc, char **argv)
{
	int rc = 0;
	int i, nThread = 0;
	char *tmp;
	int type = 0;
	int clients;
	type = PROCESS;
	dp.fname = NULL;
	dp.verbose = 0;
	int server = 1;
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
			server = 0;
			continue;
		}
		printf("Ignoring unknown option: %s\n", argv[i]);
	}

	if (server) {
		if (!(dp.fname && nThread))
			usage();
		if (clients > 0) {
			configureServer(clients);
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
	finish = finishProcess;
	LIST_RESULTS = LIST_RESULTS_PROCESS;
	initialize(nThread);
	if (server) {
		rc = master();
	} else {
		masterClient();
		free(dp.fname);
	}
	clean();

	return rc;
}
