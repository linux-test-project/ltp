/*
 * Softdog 1.21
 * Copyright (c) Christophe Dupre  1998
 *
 * A software watchdog driver for the Linux system.  Based on Alan Cox's 
 * driver, but modified to run as a daemon. Based on work done by Jamey Graham.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <linux/fs.h>

#define VERSION "1.21"
#define MAX_DEVICE_FILE_LENGTH 256

#ifndef DEVICE
# define DEVICE "/dev/watchdog"
#endif
#ifndef MY_MAJOR
# define MY_MAJOR 10
#endif
#ifndef MY_MINOR
# define MY_MINOR 130
#endif

#define SLEEP_DELAY  20
#define LOCKFILE "/var/run/watchdog.pid"
#define PROC_MISC "/proc/misc"
#define SOFTDOG20 "softdog"
#define SOFTDOG22 "watchdog"
#define TESTFILE "/tmp/watchdog.test"
#define ADDTESTCOUNT	6

/* function prototypes */
void print_help(char **argv);
void print_version(void);
int kill_daemon(void);
int proc_misc_check_device(void);

int main(int argc, char *argv[]) {     
	int timerfd;
	FILE *lockfd;
	pid_t pid;
	int curopt;
	int chldpid;
	int testfilefd;
	int not_loaded=0;

	while(1) {  /* Option parsing routine */
	      int longopt_index = 0;   /* Used only to satisfy getopt_long() */
	      struct option long_options[] =
	      {
		    {"help",0,0,'h'},
		    {"version",0,0,'v'},
		    {"kill",0,0,'k'},
		    {0,0,0,0}
	      };
	      curopt = getopt_long(argc, argv, "hvk", long_options, &longopt_index);
	      if(curopt < 0)
			break;
	      switch(curopt) {
	      case 'h':
			print_help(argv);
			exit(EXIT_SUCCESS);
	      case 'v':
			print_version();
			exit(EXIT_SUCCESS);
	      case 'k':
			if(kill_daemon() < 0) {
			  fprintf(stderr, "%s: could not kill daemon\n", argv[0]);
			  exit(EXIT_FAILURE);
			}
			printf("%s: watchdog driver successfully unloaded\n", argv[0]);
			exit(EXIT_SUCCESS);
	      case '?':
			print_help(argv);
			exit(EXIT_FAILURE);
	      default:
			break;  
	      }
	}
	if(optind != argc) {  /* additional bad arguments on line */
	      fprintf(stderr,"%s: Illegal option -- %s\n",argv[0], argv[optind]);
	      print_help(argv);
	      exit(EXIT_FAILURE);
	}

	/*
	 * make sure softdog is installed as part of the kernel. 
	 */
	if(proc_misc_check_device() < 0) {
	    fprintf(stderr,"%s: software watchdog not installed in kernel\n",argv[0]);
	    fprintf(stderr,"If kerneld is not running and is not properly configured, SOFTDOG \n"
	    	"will fail. Please read the README file in the SOFTDOG distribution.\n\n");
	    not_loaded=1;
	}

	/*
	 * Since we've already guaranteed the kernel device is present above, any
	 * open failures means that /dev/watchdog is already open (which means
	 * that some timer already has control of it). We do this BEFORE the fork
	 * so our LOCKFILE doesn't get written with the wrong pid.
	 *
	 * Check if /dev/watchdog exists, and if not create it.
	 */
	if ((timerfd = open(DEVICE, O_WRONLY)) < 0) {
		if (errno == ENOENT)
		/* Open failed because /dev/watchdog do not exist - try to create it */
		{
			if (mknod(DEVICE, S_IFCHR | 0600, MKDEV(MY_MAJOR,MY_MINOR)))
			{
				/* device node creation failed */
				fprintf(stderr,"%s: could not open %s.  Daemon may already be running\n", 
				      argv[0], DEVICE);
		      		exit(EXIT_FAILURE);
			}
			if ((timerfd = open(DEVICE, O_WRONLY)) < 0) 
			{
				/* device creation succeeded, by open failed again */
			      fprintf(stderr,"%s: could not open %s.  Daemon may already be running\n", 
				      argv[0], DEVICE);
		      	      exit(EXIT_FAILURE);

			}
		} else {
		      fprintf(stderr,"%s: could not open %s.  Daemon may already be running\n", 
			      argv[0], DEVICE);
		      exit(EXIT_FAILURE);
		}
	}
	if (not_loaded == 1) 
	{
		fprintf(stderr, "OK, kerneld loaded the module, continuing...\n\n");
	}

#ifndef DEBUG
	/*
	 * Fork process and detatch
	 */  
	if ((pid = fork()) > 0) { 
	      lockfd = fopen(LOCKFILE, "w");
	      fprintf(lockfd, "%d", pid);
	      fclose(lockfd);
	      exit(0);
	}
	else if(pid < 0) {
	      perror("fork");
	      fprintf(stderr,"%s: could not start daemon\n",argv[0]);
	      exit(EXIT_FAILURE);
	}
	if(setsid() < 0) {
	      perror("setsid");
	      fprintf(stderr, "%s: could not start softdog driver\n", argv[0]);
	      exit(EXIT_FAILURE);
	}
  
        /* Ignore unneeded signals */
#ifdef SIGTTOU
        signal(SIGTTOU, SIG_IGN);
#endif
#ifdef SIGTTIN
        signal(SIGTTIN, SIG_IGN);
#endif
#ifdef SIGCHLD
        signal(SIGCHLD, SIG_IGN);
#endif
#ifdef SIGHUP
        signal(SIGHUP, SIG_IGN);
#endif
#ifdef SIGINT
        signal(SIGINT, SIG_IGN);
#endif
#ifdef SIGQUIT
        signal(SIGQUIT, SIG_IGN);
#endif
#ifdef SIGTSTP
        signal(SIGTSTP, SIG_IGN);
#endif
#ifdef SIGUSR1
        signal(SIGUSR1, SIG_IGN);
#endif
#ifdef SIGUSR2
        signal(SIGUSR2, SIG_IGN);
#endif
  
	/* Close all unneeded file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* Change current directory to / so that we do not prevent 
	 * filesystem unmounting 
	 */
	chdir("/");

	umask(0);
	
#endif /* DEBUG */

	/*
	 * Now just start the timer and keep it running
	 */
	while(1) {
#ifdef DEBUG    			  
    		fprintf(stderr, "Begin loop\n");
#endif
    	/* Check if we can still fork */
#ifdef DEBUG    			  
    		fprintf(stderr, "fork()ing...\n");
#endif
	    	if ((chldpid=fork()) == -1) {
#ifdef DEBUG    			  
    			fprintf(stderr, "Could not fork()\n");
#endif
    			sleep(SLEEP_DELAY/3);
	    		continue; /* Could not fork - do not reset the watchdog timer */
    		} 
	    	else if (chldpid == 0) exit(0); /* child - let's exit */
    		else {	/* parent - check if we can open a file */
#ifdef DEBUG    			  
    			fprintf(stderr, "open()ing...\n");
#endif
    			testfilefd = open(TESTFILE, O_RDONLY|O_CREAT);
	    		if (testfilefd != -1) {
    				close(testfilefd);	/* this should not fail */
    				unlink(TESTFILE);	/* this should not fail */
	    		} 
    			else if ((errno == EMFILE) || (errno == ENFILE) || 
    				(errno == ENOMEM)) {
#ifdef DEBUG    			  
	    			fprintf(stderr, "Could not open()\n");
#endif
	    			sleep(SLEEP_DELAY/3);
    				continue; /* Could not open file - do not reset 
    					   * the watchdog timer 
    					   */
	    		}
    		}

		write(timerfd, "\0", 1);
#ifdef DEBUG    			  
		fprintf(stderr, "Watchdog timer reset\n");
#endif
		sleep(SLEEP_DELAY);
	}
}

/*
 * Checks the /proc/misc file to see if device 'device' with minor
 * number 'minor' exists.  Returns 0 on success (device installed)
 * and -1 on failure (device not installed or minor number differs).
 */

int proc_misc_check_device(void) 
{
      FILE *procfd;
      int thisminor;
      char thisdevice[MAX_DEVICE_FILE_LENGTH];

      if((procfd = fopen(PROC_MISC, "r")) == NULL)
		return -1;
      while(fscanf(procfd,"%d %s", &thisminor, thisdevice) > 0) {
	    if(thisminor == MY_MINOR) {
		  if(strcmp(thisdevice, SOFTDOG20) == 0) {
			fclose(procfd);
			return 0;
		  }
		  else if(strcmp(thisdevice, SOFTDOG22) == 0) {
			fclose(procfd);
			return 0;
		  }
	    }
      }
      fclose(procfd);
      return -1;  /* match didn't occur */
}

/*
 * kills the running daemon process.  Returns 0 if it succeeds, or -1 if
 * it fails
 */
int
kill_daemon(void)
{
      FILE *lockfd;
      pid_t pid;

      if((lockfd = fopen(LOCKFILE, "r")) == NULL) {
	    fprintf(stderr,"cannot find %s\n", LOCKFILE);
	    return -1;
      }

      if(fscanf(lockfd,"%d", &pid) < 1) {
	    fprintf(stderr,"cannot read %s\n", LOCKFILE);
	    return -1;
      }

      if(kill(pid, SIGKILL) < 0) {
	    perror("process");
	    return -1;
      }
      unlink(LOCKFILE); /* this should never fail */
      return 0;
}

/*
 * Help and version screens
 */
void
print_help(char **argv) {
  printf("\
Usage: %s [-khv]\n\
  -k, --kill    : kill running %s process\n\
  -h, --help    : this help screen\n\
  -v, --version : version information\n",argv[0], argv[0]);
}

void
print_version(void) {
  	printf("Softdog version %s\n(C) Christophe Dupre 1997,1998,1999\n", VERSION);
}

