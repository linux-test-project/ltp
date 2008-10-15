
LOADLIBES += -lm
LFLAGS += -lw
CFLAGS += -w 
DESTDIR = /opt/ltp

all: pan bump scanner

pan: pan.o zoolib.o splitstr.o


bump: bump.o zoolib.o


scanner: scan.o scanner.o reporter.o tag_report.o symbol.o splitstr.o debug.o


install: pan bump scanner
	install -D scanner $(DESTDIR)/bin/scanner
	install -D pan $(DESTDIR)/bin/pan
	install -D bump $(DESTDIR)/bin/bump

clean:
	rm -f *.o pan bump scanner
