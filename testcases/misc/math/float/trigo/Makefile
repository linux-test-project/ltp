CFLAGS += -Wall
LOADLIBES = -lm

BIN = gensin gencos gentan genatan genatan2 genacos genasin gentrigo
DATA = dtan rtan datan ratan dsin rsin datan2 ratan2 dcos rcos dasin rasin dacos racos

all:	$(BIN)


clean:
	rm -f $(BIN)
	@set -e; for i in $(BIN); do rm -f ../../../../bin/$$i ; done
	rm -f $(DATA)
	@set -e; for i in $(DATA); do rm -f ../../../../bin/$$i ; done

install:
	@set -e; for i in $(BIN); do ln -f $$i ../../../../bin/$$i ; done
