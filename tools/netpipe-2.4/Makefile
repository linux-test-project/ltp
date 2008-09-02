#
# Makefile for NetPIPE
#

# Default C compiler: must be an ANSI C compiler
#CC         = cc
# File names for the main source files
DRIV_SRC   = netpipe.c
DRIV_OBJ   = netpipe.o
INCLUDES   = netpipe.h
# Default target is just TCP
TARGETS    = NPtcp
# If you have TCP, MPI and PVM
#TARGETS    = NPtcp NPmpi NPpvm
CFLAGS		    += -O -Wall
# Adjust these for MPI (only used if you have MPI)
MPI_HOME   = /home/mpich
MPI_ARCH   = IRIX
MPI_DEVICE = ch_p4
# Adjust these for PVM (only used if you have PVM)
PVM_HOME   = /home/pvm3
PVM_ARCH   = SUN4SOL2
# Any extra libraries needed
EXTRA_LIBS = $(LDFLAGS)
#EXTRA_LIBS = -lgcc

all:	$(TARGETS)

install:
	@/bin/chmod 755 netpipe.sh
	@ln -f $(TARGETS) netpipe.sh ../../testcases/bin/
#
# This section of the Makefile is for compiling the binaries
#

TCP:	NPtcp
	@echo 'NPtcp has been built.'

NPtcp:	NPtcp.o TCP.o
	$(CC) $(CFLAGS) NPtcp.o TCP.o  -o NPtcp $(EXTRA_LIBS)

NPtcp.o:	$(DRIV_SRC) $(INCLUDES)
	$(CC) $(CFLAGS) -DTCP -c -o NPtcp.o $(DRIV_SRC)

TCP.o:	TCP.c TCP.h $(INCLUDES)
	$(CC) $(CFLAGS) -DTCP -c TCP.c

MPI:	NPmpi
	@echo 'NPmpi has been built.'

NPmpi:	NPmpi.o MPI.o
	$(CC) $(CFLAGS) NPmpi.o MPI.o -o NPmpi  -L $(MPI_HOME)/lib/$(MPI_ARCH)/$(MPI_DEVICE) -lmpi $(EXTRA_LIBS)

NPmpi.o:	$(DRIV_SRC) $(INCLUDES) 
	$(CC) $(CFLAGS) -DMPI -I$(MPI_HOME)/include -c -o NPmpi.o $(DRIV_SRC)

MPI.o:	MPI.c MPI.h $(INCLUDES)
	$(CC) $(CFLAGS) -DMPI -I$(MPI_HOME)/include -c MPI.c

PVM:	NPpvm
	@echo 'NPpvm has been built.'

NPpvm:	NPpvm.o PVM.o
	$(CC) $(CFLAGS) NPpvm.o PVM.o -o NPpvm  -L$(PVM_HOME)/lib/$(PVM_ARCH)/ -lpvm3 $(EXTRA_LIBS)

NPpvm.o:	$(DRIV_SRC) $(INCLUDES)
	$(CC) $(CFLAGS) -DPVM -I$(PVM_HOME)/include -c -o NPpvm.o $(DRIV_SRC)

PVM.o:	PVM.c PVM.h $(INCLUDES)
	$(CC) $(CFLAGS) -DPVM -I$(PVM_HOME)/include -c PVM.c

clean:
	rm -f $(TARGETS) *.o core a.out
