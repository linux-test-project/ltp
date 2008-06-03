.PHONY: all clean install

LTP_SRC_ROOT=../../../../..
LTP_BIN=$(LTP_SRC_ROOT)/testcases/bin
#CFLAGS+= -I$(LTP_SRC_ROOT)/include -Wall -g
CFLAGS+= -Wall -g
#LIBS+= -L$(LTP_SRC_ROOT)/lib -lltp
LIBS+=

all: smount nsclone

install: all
	#@ln -f nsclone $(LTP_BIN)/nsclone
	#@ln -f smount $(LTP_BIN)/smount
	@/bin/true

uninstall:
	#rm -f $(LTP_BIN)/nsclone $(LTP_BIN)/smount
	@/bin/true

nsclone: nsclone.c
	$(CC) $(CFLAGS) $(LIBS) -o nsclone nsclone.c

smount: smount.c
	$(CC) $(CFLAGS) $(LIBS) -o smount smount.c

clean:
	rm -f smount nsclone
