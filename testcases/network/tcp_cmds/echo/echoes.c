/*
@! # TEST TYPE(S): Concurrency, Load stress
@! # TESTCASE DESCRIPTION:
@! # 	Purpose: to send packets from the file to echo protocol on remote
@! #		 machine and read the echoing packets back and compare them
@! # 	Design: Connect to echo protocol on the remote machine
@! #		read from the file and send the file to remote machine
@! #		read the echoing  packets and store them in a file
@! #		repeat until file exhausted.
@! #		compare result
@! #
@! # SPEC. EXEC. REQS: May require multiple of this test to run
@! #		       to target machines from multiple machine in order
@! #		       to create stress condition
@! # 			echoes <REMOTE HOST> <echofile> <number of process>
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

void echofile (struct servent *, struct hostent *, char *, char *);
void getfilename(char *, char*, int);
int checkfile(char *, char *);
void itoa(int, char []);
void reverse(char []);
void cleanup(int);


int main (int argc,char *argv[],char *env[])
{

	int	i,j,k,wait_stat,pid,finish;
	struct	servent *sp;
	struct 	hostent *hp;
	struct  {
		char	resultfile[35];
		int	pid;
		int	status;
	}echo_struc[200];

	hp = gethostbyname(argv[1]);
	if ((sp=getservbyname("echo","tcp"))==NULL) {
		printf("ERROR service is not available\n");
		perror("echo");
		exit(1);
	}
	i=atoi(argv[3]);
	j=0;
	while ( i-- > 0 )  {
		echo_struc[j].status=0;
		switch (pid=fork())  {
		case 0:
			getfilename(argv[2],echo_struc[j].resultfile,j);
			echofile(sp,hp,echo_struc[j].resultfile,argv[2]); 
			break;
		case -1:
			printf("ERROR when forking a new process\n");
			perror("echo");
			exit(1);
			break;
		default:
			echo_struc[j].pid=pid;
			echo_struc[j].status=1;
			j++;
			break;
		}
	}
	finish=atoi(argv[3]);
	i=finish;
	while (finish != 0) {
		if ((pid=wait(&wait_stat)) == -1) {
			printf("ERROR in wait process\n");
			perror("echo");
			exit(1);
		}
		if (wait_stat==0) 
			for (j=0;j<i;j++) {
				if(echo_struc[j].pid==pid) {
					echo_struc[j].status=2;
					finish--;
					j=i;
				}
			}
		else {
			for (k=0;k < i;k++)
				if (echo_struc[k].status==1) {
					kill(echo_struc[k].pid,9);
				}
			exit(1);
		}
	}
	exit(0);
}

void echofile (struct servent *sp, struct hostent *hp, char *resultfile, char *orgfile)
{
	int	n;
	int	port;
	char	wr_buffer[BUFSIZ];
	char	rd_buffer[BUFSIZ];
	struct	in_addr	hostaddr;
	struct sockaddr_in sa;
#ifdef DEBUG
	struct	sockaddr_in address;
	u_short	addrlen;
	u_short	portnum;
#endif
	int	s;
	int	finish;
	int	fdw,fdr;
	int	nread,nwrite;
	int	pid;
	int	count;
#ifdef 	DEBUG
	printf("Create socket .....\n");
#endif
	pid=getpid();
	if ((s=socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
		printf("ERROR occured during socket operation(%d)\n",pid);
		perror("echo:socket");
		cleanup(s);
		exit(1);
	}
	port=sp->s_port;
	memcpy(&hostaddr,hp->h_addr_list[0],sizeof(struct in_addr));
	memset((char *)&sa,0x00,sizeof (sa));
	sa.sin_port=port;
	sa.sin_family=AF_INET;
	sa.sin_addr=hostaddr;

#ifdef 	DEBUG
	printf("sizeof (hostaddr)=%d\n",sizeof (hostaddr));
	printf("port=%d hostaddr=%x", ntohs(port), hostaddr);
	printf("Connect .......\n");
#endif
	if (connect(s,(struct sockaddr *) &sa,sizeof(sa))==-1) {
		printf ("ERROR occured during connect socket operation(%d)\n",pid);
		perror("echo:connect");
		cleanup(s);
		exit(1);
	}
#ifdef DEBUG
	addrlen=sizeof(address);
	printf("addrlen=%d\n",addrlen);
	printf("hp->h_length=%d\n",hp->h_length);
	printf("hp->h_addrtype=%d\n",hp->h_addrtype);
	printf("hp->h_addr=%d\n",inet_ntoa(hp->h_addr));
	if (getsockname(s,&address,&addrlen) == -1 ) {
		printf ("ERROR occured during getsockname(%d)\n",pid);
		perror("echo");
		cleanup(s);
		exit(1);
	}
	portnum=ntohs(address.sin_port);
	printf ("local port is: %d\n",portnum);
	if (getpeername(s,&address,&addrlen) == -1) {
		printf ("ERROR occured during getpeername(%d)\n",pid);
		perror("echo");
		cleanup(s);
		exit(1);
	}
	portnum=ntohs(address.sin_port);
	/*	printf ("remote address is: %d\n",portnum);
*/
#endif
	if ((fdr=open(orgfile,O_RDONLY)) < 0 ) {
		printf("ERROR when opening the input file(%d)\n",pid);
		perror("echo:orginal file");
		cleanup(s);
		exit(1);
	}
	if ((fdw=creat(resultfile,0644)) < 0 ) {
		printf("ERROR when opening the temporary temp file(%d)\n",pid);
		perror("echo:resultfile");
		cleanup(s);
		exit(1);
	}
	finish=FALSE;
	count=0;
	while (!finish) {
		if ((nwrite=read(fdr,wr_buffer,BUFSIZ))==-1) {
			printf("ERROR when reading input file(%d)\n",pid);
			perror("echo:orginal file");
			cleanup(s);
			exit(1);
		}
		if (nwrite==0)
			finish=TRUE;
		else {
			count++;
			if((n=write(s,wr_buffer,nwrite))!=nwrite) {
				printf("ERROR during write to socket(%d)\n",pid);
				perror("echo:socket write");
				cleanup(s);
				exit(1);
			}
#ifdef 	DEBUG
/*
			printf("Writing .......%d\n",count);
*/
#endif
			while (nwrite!=0) {
				if((nread=read(s,rd_buffer,BUFSIZ))==-1) {
					printf("read size:%d\n",n);
					printf("ERROR during read from socket(%d)\n",pid);
					perror("echo:socket read");
					cleanup(s);
					exit(1);
				}
#ifdef 	DEBUG
/*
				printf("Reading .......    %d\n",count);
*/
#endif
				if((n=write(fdw,rd_buffer,nread))!=nread) {
					printf("read size:%d\n",n);
					printf("ERROR during write to result file(%d)\n",pid);
					perror("echo:result file");
					cleanup(s);
					exit(1);
				}
				nwrite-=nread;
			}

		}/* end of else */
	} /* end of while */
	if ((n=close(s)) == -1) {
		printf("ERROR in closing socket(%d)\n",pid);
		perror("echo");
		exit(1);
	}
	if ((n=close(fdr)) == -1) {
		printf("ERROR in closing input file(%d)\n",pid);
		perror("echo");
		exit(1);
	}
	if ((n=close(fdw) ) == -1) {
		printf("ERROR in closing temp file(%d)\n",pid);
		perror("echo");
		exit(1);
	}
	if (checkfile(orgfile,resultfile)) {
		printf("ERROR input file and output file are not equal(%d)\n",pid);
		exit(1);
	}
	printf("Finish ....%d\n",pid);
	exit(0);
}

void getfilename(char *strptr, char* filename, int j)
{
	int 	i;
	char	s[10],*sptr=&s[0];

	strcpy(filename,strptr);
	itoa(j,s);
	strcat(filename,s);
}

int checkfile(char *file1, char *file2)
{
	int	n;
	struct	stat buffer,*bufptr=&buffer;
	stat(file1,bufptr);
	n=bufptr->st_size;
#ifdef 	DEBUG
	/*	printf("%s size= %d \n",file1,n);
*/
#endif
	stat(file2,bufptr);
#ifdef 	DEBUG
	/*	printf("%s size= %d \n",file2,bufptr->st_size);
*/
#endif
	if(n != buffer.st_size)
		return(TRUE);
	else return(FALSE);
}

void itoa(int n, char s[])
{
	int i, sign;

	if ((sign = n) < 0)
		n = -n;
	i = 0;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

void reverse(char s[])
{
	int c, i, j;

	for (i=0, j=strlen(s)-1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

void cleanup(int s) 
{
	close(s);
}
