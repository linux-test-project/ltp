CPPFLAGS+=	-I../../include
LDLIBS+=	-lpthread

TARGETS=	sem_lock.test sem_conpro.test sem_readerwriter.test \
		sem_philosopher.test sem_sleepingbarber.test

all: $(TARGETS)

%.test: %.o
	$(LINK.o) $< $(LOADLIBES) $(LDLIBS) -o $@

clean: 
	rm -f $(TARGETS)

test:
	./run.sh
