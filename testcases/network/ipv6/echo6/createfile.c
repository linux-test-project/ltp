/*
@! # TESTCASE DESCRIPTION:
@! # 	Purpose: to create an input file of any size
@! # 	Command: none
@! # 	Subcommand: none
@! # 	Design: Write an array the size of BUFSIZ to created file until
@! # 		the file size matches the file size required
@! # SPEC. EXEC. REQS:  This  program is used by ctatcdt3.c and ctatcet3.c
*/
#include <stdio.h>
#include <fcntl.h>
main (int argc,char *argv[]) 
{
int	fd;
int	fsize;
int	count=0;
int	n,remain;
static	char buf[BUFSIZ];
	for ( fsize=0;fsize<BUFSIZ;fsize) {
		buf[fsize++]='U';
		buf[fsize++]='\n';
	}

	fsize=atoi(argv[1]);
	if ((fd=creat(argv[2],0644)) == -1 )
		perror("createfile");
	if (fsize >= BUFSIZ) {
		count=fsize/BUFSIZ;
		remain=fsize%BUFSIZ; 
		}
	else remain=fsize;
	while (count-- !=0) 
		if((n=write(fd,buf,BUFSIZ)) != BUFSIZ) 
			perror("createfile");
	if ((n=write(fd,buf,remain)) != remain)
		perror("createfile");
	close(fd);
}
