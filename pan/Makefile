
LDLIBS = -lm
LFLAGS = -lw
CFLAGS += 

all: pan bump scanner

pan: pan.o zoolib.o splitstr.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

bump: bump.o zoolib.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

scanner: scan.o scanner.o reporter.o tag_report.o symbol.o splitstr.o debug.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o pan bump scanner
