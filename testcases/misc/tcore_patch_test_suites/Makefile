CFLAGS += -Wall
CPPFLAGS += -I../../../include
LDFLAGS += -L../../../lib
LDLIBS += -lltp -lm -lpthread
MODCFLAGS := -Wall -DLINUX -I/usr/src/linux/include
EXECS=tcore tcore.sh tcore.exp

all:		tcore

install: 
		@set -e; for i in $(EXECS) ; do ln -f $$i ../../bin/$$i ; done ;	

clean:
	rm -f tcore

