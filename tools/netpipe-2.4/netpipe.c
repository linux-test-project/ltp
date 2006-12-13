/*****************************************************************************/
/* "NetPIPE" -- Network Protocol Independent Performance Evaluator.          */
/* Copyright 1997, 1998 Iowa State University Research Foundation, Inc.      */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation.  You should have received a copy of the     */
/* GNU General Public License along with this program; if not, write to the  */
/* Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.   */
/*                                                                           */
/* Files needed for use:                                                     */
/*     * netpipe.c          ---- Driver source                               */
/*     * netpipe.h          ---- General include file                        */
/*     * TCP.c              ---- TCP calls source                            */
/*     * TCP.h              ---- Include file for TCP calls and data structs */
/*     * MPI.c              ---- MPI calls source                            */
/*     * MPI.h              ---- Include file for MPI calls and data structs */
/*     * PVM.c              ---- PVM calls source                            */
/*     * PVM.h              ---- Include file for PVM calls and data structs */
/*****************************************************************************/
#include "netpipe.h"

extern char *optarg;

int
main(int argc, char *argv[])
{
    FILE        *out;           /* Output data file                          */
    char        s[255];         /* Generic string                            */
    char        *memtmp;
    char        *memtmp1;

    int         c,              /* option index                              */
                i, j, n, nq,    /* Loop indices                              */
		asyncReceive=0, /* Pre-post a receive buffer?                */
                bufoffset=0,    /* Align buffer to this                      */
                bufalign=16*1024,/* Boundary to align buffer to              */
		errFlag,        /* Error occurred in inner testing loop      */
                nrepeat,        /* Number of time to do the transmission     */
                len,            /* Number of bytes to be transmitted         */
                inc=0,          /* Increment value                           */
                trans=-1,       /* Transmitter flag. 1 if transmitting.      */
                detailflag=0,   /* Set to examine the signature curve detail */
                bufszflag=0,    /* Set to change the TCP socket buffer size  */
                pert,           /* Perturbation value                        */
                start=1,        /* Starting value for signature curve        */
                end=MAXINT,     /* Ending value for signature curve          */
                streamopt=0,    /* Streaming mode flag                       */
                printopt=0;     /* Debug print statements flag               */
   
    ArgStruct   args;           /* Argumentsfor all the calls                */

    double      t, t0, t1, t2,  /* Time variables                            */
                tlast,          /* Time for the last transmission            */
                latency;        /* Network message latency                   */

    Data        bwdata[NSAMP];  /* Bandwidth curve data                      */

    short       port=DEFPORT;   /* Port number for connection                */
#ifdef HAVE_GETRUSAGE
    struct rusage prev_rusage, curr_rusage; /* Resource usage                */
    double	user_time, sys_time;	/* User & system time used                   */
    double	best_user_time, best_sys_time; /* Total user & system time used     */
    double      ut1, ut2, st1, st2; /* User & system time ctrs for variance  */
    double	ut_var, st_var;	/* Variance in user & system time            */
#endif

#ifdef MPI
    MPI_Init(&argc, &argv);
#endif

    strcpy(s, "NetPIPE.out");
#ifndef MPI
    if(argc < 2)
     PrintUsage();
#endif

    /* Parse the arguments. See Usage for description */
    while ((c = getopt(argc, argv, "Pstrh:p:o:A:O:l:u:i:b:a")) != -1)
    {
        switch(c)
        {
            case 'o': strcpy(s,optarg);
                      break;

            case 't': trans = 1;
                      break;
            
            case 'r': trans = 0;
                      break;

            case 's': streamopt = 1;
                      break;

            case 'l': /*detailflag = 1;*/
                      start = atoi(optarg);
                      if (start < 1)
                      {
                        fprintf(stderr,"Need a starting value >= 1\n");
                        exit(743);
                      }
                      break;

            case 'u': /*detailflag = 1;*/
                      end = atoi(optarg);
                      break;

            case 'i': detailflag = 1;
                      inc = atoi(optarg);
                      break;

            case 'b': bufszflag = 1;
#ifdef TCP
                      args.prot.rcvbufsz=atoi(optarg);
                      args.prot.sndbufsz=args.prot.rcvbufsz;
#endif
                      break;

            case 'P': printopt = 1;
                      break;
            
            case 'A': bufalign = atoi(optarg);
                      break;

            case 'O': bufoffset = atoi(optarg);
                      break;

            case 'p': port = atoi(optarg);
                      break;
            
            case 'h': if (trans == 1)
                      {
                          args.host = (char *)malloc(strlen(optarg)+1);
                          strcpy(args.host, optarg);
                      }
		      else
		      {
			  fprintf(stderr, "Error: -t must be specified before -h\n");
			  exit(-11);
		      }
                      break;

	    case 'a': asyncReceive = 1;
		      break;

            default:  PrintUsage(); 
                      exit(-12);
        }
    }
    if (start > end)
    {
        fprintf(stderr, "Start MUST be LESS than end\n");
        exit(420132);
    }
#if defined(TCP) || defined(PVM)
    /*
      It should be explicitly specified whether this is the transmitter
      or the receiver.
    */
    if (trans < 0)
    {
	fprintf(stderr, "Error: either -t or -r must be specified\n");
	exit(-11);
    }
#endif

    args.nbuff = TRIALS;
    args.tr = trans;
    args.port = port;

#if defined(TCP)
    if (!bufszflag)
    {
        args.prot.sndbufsz = 0;
        args.prot.rcvbufsz = 0;
    }
    else
        fprintf(stderr,"Send and Recv Buffers are %d bytes\n",
                                                   args.prot.sndbufsz);
#endif

    Setup(&args);
    Establish(&args);

    if (args.tr)
    {
        if ((out = fopen(s, "w")) == NULL)
        {
            fprintf(stderr,"Can't open %s for output\n", s);
            exit(1);
        }
    }
    else
        out = stdout;

    args.bufflen = 1;
    args.buff = (char *)malloc(args.bufflen);
    args.buff1 = (char *)malloc(args.bufflen);
    if (asyncReceive)
	PrepareToReceive(&args);
    Sync(&args);
    t0 = When();
    t0 = When();
    t0 = When();
#ifdef HAVE_GETRUSAGE
    getrusage(RUSAGE_SELF, &prev_rusage);
#endif
    t0 = When();
    for (i = 0; i < LATENCYREPS; i++)
    {
        if (args.tr)
        {
            SendData(&args);
            RecvData(&args);
	    if (asyncReceive && (i < LATENCYREPS - 1))
	    {
		PrepareToReceive(&args);
	    }
        }
        else
        {
            RecvData(&args);
	    if (asyncReceive && (i < LATENCYREPS - 1))
	    {
		PrepareToReceive(&args);
	    }
            SendData(&args);
        }
    }
    latency = (When() - t0)/(2 * LATENCYREPS);
#ifdef HAVE_GETRUSAGE
    getrusage(RUSAGE_SELF, &curr_rusage);
#endif
    free(args.buff);
    free(args.buff1);


    if (args.tr)
    {
        SendTime(&args, &latency);
    }
    else
    {
        RecvTime(&args, &latency);
    }
    if (args.tr && printopt)
    {
        fprintf(stderr,"Latency: %.7f\n", latency);
        fprintf(stderr,"Now starting main loop\n");
    }
    tlast = latency;
    if (inc == 0)
    {
	/* Set a starting value for the message size increment. */
	inc = (start > 1) ? start / 2 : 1;
    }

    /* Main loop of benchmark */
    for (nq = n = 0, len = start, errFlag = 0; 
         n < NSAMP - 3 && tlast < STOPTM && len <= end && !errFlag; 
         len = len + inc, nq++ )
    {
        if (nq > 2 && !detailflag)
	  {
	    /*
	      This has the effect of exponentially increasing the block
	      size.  If detailflag is false, then the block size is
	      linearly increased (the increment is not adjusted).
	    */
            inc = ((nq % 2))? inc + inc: inc;
	  }
        
        /* This is a perturbation loop to test nearby values */
        for (pert = (!detailflag && inc > PERT+1)? -PERT: 0;
             pert <= PERT; 
             n++, pert += (!detailflag && inc > PERT+1)? PERT: PERT+1)
        {

            /* Calculate how many times to repeat the experiment. */
            if (args.tr)
            {
                nrepeat = MAX((RUNTM / ((double)args.bufflen /
                                 (args.bufflen - inc + 1.0) * tlast)), TRIALS);
                SendRepeat(&args, nrepeat);
            }
            else
            {
                RecvRepeat(&args, &nrepeat);
            }

            /* Allocate the buffer */
            args.bufflen = len + pert;
            if((args.buff=(char *)malloc(args.bufflen+bufalign))==(char *)NULL)
            {
                fprintf(stderr,"Couldn't allocate memory\n");
		errFlag = -1;
                break;
            }
            if((args.buff1=(char *)malloc(args.bufflen+bufalign))==(char *)NULL)
            {
                fprintf(stderr,"Couldn't allocate memory\n");
		errFlag = -1;
                break;
            }
            /*
	      Possibly align the data buffer: make memtmp and memtmp1
	      point to the original blocks (so they can be freed later),
	      then adjust args.buff and args.buff1 if the user requested it.
	    */
            memtmp = args.buff;
            memtmp1 = args.buff1;
            if (bufalign != 0)
                args.buff +=(bufalign - 
                        ((int)(*args.buff) % bufalign) + bufoffset) % bufalign;

            if (bufalign != 0)
                args.buff1 +=(bufalign - 
                        ((int)(*args.buff1) % bufalign) + bufoffset) % bufalign;


            if (args.tr && printopt)
                fprintf(stderr,"%3d: %9d bytes %4d times --> ",
                                 n,args.bufflen,nrepeat);

            /* Finally, we get to transmit or receive and time */
            if (args.tr)
            {
		/*
		   This is the transmitter: send the block TRIALS times, and
		   if we are not streaming, expect the receiver to return each
		   block.
		*/
                bwdata[n].t = LONGTIME;
                t2 = t1 = 0;
#ifdef HAVE_GETRUSAGE
		ut1 = ut2 = st1 = st2 = 0.0;
		best_user_time = best_sys_time = LONGTIME;
#endif
                for (i = 0; i < TRIALS; i++)
                {
                    Sync(&args);
#ifdef HAVE_GETRUSAGE
		    getrusage(RUSAGE_SELF, &prev_rusage);
#endif
                    t0 = When();
                    for (j = 0; j < nrepeat; j++)
                    {
			if (asyncReceive && !streamopt)
			{
			    PrepareToReceive(&args);
			}
                        SendData(&args);
                        if (!streamopt)
			{
                            RecvData(&args);
			}
                    }
                    t = (When() - t0)/((1 + !streamopt) * nrepeat);
#ifdef HAVE_GETRUSAGE
		    getrusage(RUSAGE_SELF, &curr_rusage);
		    user_time = ((curr_rusage.ru_utime.tv_sec -
			      prev_rusage.ru_utime.tv_sec) + (double)
			     (curr_rusage.ru_utime.tv_usec -
			      prev_rusage.ru_utime.tv_usec) * 1.0E-6) /
		      ((1 + !streamopt) * nrepeat);
		    sys_time = ((curr_rusage.ru_stime.tv_sec -
			      prev_rusage.ru_stime.tv_sec) + (double)
			     (curr_rusage.ru_stime.tv_usec -
			      prev_rusage.ru_stime.tv_usec) * 1.0E-6) /
		      ((1 + !streamopt) * nrepeat);
		    ut2 += user_time * user_time;
		    st2 += sys_time * sys_time;
		    ut1 += user_time;
		    st1 += sys_time;
		    if ((user_time + sys_time) < (best_user_time + best_sys_time))
		    {
			best_user_time = user_time;
			best_sys_time = sys_time;
		    }
#endif

                    if (!streamopt)
                    {
                        t2 += t*t;
                        t1 += t;
                        bwdata[n].t = MIN(bwdata[n].t, t);
                    }
                }
                if (!streamopt)
                    SendTime(&args, &bwdata[n].t);
                else
                    RecvTime(&args, &bwdata[n].t);

                if (!streamopt)
                    bwdata[n].variance = t2/TRIALS - t1/TRIALS * t1/TRIALS;

#ifdef HAVE_GETRUSAGE
		ut_var = ut2/TRIALS - (ut1/TRIALS) * (ut1/TRIALS);
		st_var = st2/TRIALS - (st1/TRIALS) * (st1/TRIALS);
#endif

            }
            else
            {
		/*
		   This is the receiver: receive the block TRIALS times, and
		   if we are not streaming, send the block back to the
		   sender.
		*/
                bwdata[n].t = LONGTIME;
                t2 = t1 = 0;
                for (i = 0; i < TRIALS; i++)
                {
		    if (asyncReceive)
		    {
		    	PrepareToReceive(&args);
		    }
                    Sync(&args);
                    t0 = When();
                    for (j = 0; j < nrepeat; j++)
                    {
                        RecvData(&args);
			if (asyncReceive && (j < nrepeat - 1))
			{
			    PrepareToReceive(&args);
			}
                        if (!streamopt)
                            SendData(&args);
                    }
                    t = (When() - t0)/((1 + !streamopt) * nrepeat);

                    if (streamopt)
                    {
                        t2 += t*t;
                        t1 += t;
                        bwdata[n].t = MIN(bwdata[n].t, t);
                    }
                }
                if (streamopt)
                    SendTime(&args, &bwdata[n].t);
                else
                    RecvTime(&args, &bwdata[n].t);

                if (streamopt)
                    bwdata[n].variance = t2/TRIALS - t1/TRIALS * t1/TRIALS;

            }
            tlast = bwdata[n].t;
            bwdata[n].bits = args.bufflen * CHARSIZE;
            bwdata[n].bps = bwdata[n].bits / (bwdata[n].t * 1024 * 1024);
            bwdata[n].repeat = nrepeat;
            
            if (args.tr)
	    {
		fprintf(out, "%.7f %.7f %d %d %.7f", bwdata[n].t, bwdata[n].bps,
			bwdata[n].bits, bwdata[n].bits / 8,
			bwdata[n].variance);
#ifdef HAVE_GETRUSAGE
		fprintf(out, " %.7f %.7f %.7f %.7f", ut1 / (double) TRIALS,
			st1 / (double) TRIALS, ut_var, st_var);
#endif
		fprintf(out, "\n");
	    }
            fflush(out);

            free(memtmp);
            free(memtmp1);

            if (args.tr && printopt)
	    {
        	fprintf(stderr," %6.2f Mbps in %.7f sec", bwdata[n].bps,
			tlast);
#ifdef HAVE_GETRUSAGE
		fprintf(stderr, ", avg utime=%.7f avg stime=%.7f, ",
			ut1 / (double) TRIALS,
			st1 / (double) TRIALS);
		fprintf(stderr, "min utime=%.7f stime=%.7f, ", best_user_time,
			best_sys_time);
		fprintf(stderr, "utime var=%.7f stime var=%.7f", ut_var, st_var);
#endif
		fprintf(stderr, "\n");
	    }
        } /* End of perturbation loop */

    } /* End of main loop  */
        
    if (args.tr)
       fclose(out);
         
    CleanUp(&args);
    return(0);
}


/* Return the current time in seconds, using a double precision number.      */
double
When()
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return ((double) tp.tv_sec + (double) tp.tv_usec * 1e-6);
}

void
PrintUsage(void)
{
	printf("\n NETPIPE USAGE \n\n");
	printf("A: specify buffers alignment e.g.: <-A 1024>\n");
	printf("a: asynchronous receive (a.k.a. preposted receive)\n");
#if defined(TCP)
	printf("b: specify send and receive buffer sizes e.g. <-b 32768>\n");
	printf("h: specify hostname <-h host>\n");
#endif
	printf("i: specify increment step size e.g. <-i 64>\n");
	printf("l: lower bound start value e.g. <-i 1>\n");
	printf("O: specify buffer offset e.g. <-O 127>\n");
	printf("o: specify output filename <-o fn>\n");
	printf("P: print on screen\n");
#if defined(TCP)
	printf("p: specify port e.g. <-p 5150>\n");
#endif
	printf("r: receiver\n");
	printf("s: stream option\n");
	printf("t: transmitter\n");
	printf("u: upper bound stop value e.g. <-u 1048576>\n");
	printf("\n");
	exit(-12);
}
