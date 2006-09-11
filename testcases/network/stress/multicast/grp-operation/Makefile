SCRIPTS = mcast4-grpope01 mcast4-grpope02 mcast4-grpope03 mcast4-grpope04 \
	  mcast6-grpope01 mcast6-grpope02 mcast6-grpope03 mcast6-grpope04

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
