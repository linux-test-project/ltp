
LOADLIBES += -lm
LFLAGS += -l -w
CFLAGS += -w 
PREFIX = /opt/ltp

all: ltp-pan ltp-bump ltp-scanner

ltp-pan: ltp-pan.o zoolib.o splitstr.o


ltp-bump: ltp-bump.o zoolib.o


ltp-scanner: scan.o ltp-scanner.o reporter.o tag_report.o symbol.o splitstr.o debug.o


install: ltp-pan ltp-bump ltp-scanner
	install -D ltp-scanner $(DESTDIR)/$(PREFIX)/bin/ltp-scanner
	install -D ltp-pan $(DESTDIR)/$(PREFIX)/bin/ltp-pan
	install -D ltp-bump $(DESTDIR)/$(PREFIX)/bin/ltp-bump

clean:
	rm -f *.o ltp-pan ltp-bump ltp-scanner
