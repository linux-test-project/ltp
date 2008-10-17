
LOADLIBES += -lm
LFLAGS += -l -w
CFLAGS += -w 
PREFIX = /opt/ltp

all: pan bump scanner

pan: pan.o zoolib.o splitstr.o


bump: bump.o zoolib.o


scanner: scan.o scanner.o reporter.o tag_report.o symbol.o splitstr.o debug.o


install: pan bump scanner
	install -D scanner $(DESTDIR)/$(PREFIX)/bin/scanner
	install -D pan $(DESTDIR)/$(PREFIX)/bin/pan
	install -D bump $(DESTDIR)/$(PREFIX)/bin/bump

clean:
	rm -f *.o pan bump scanner
