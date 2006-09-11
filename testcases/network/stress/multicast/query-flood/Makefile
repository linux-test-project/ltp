SCRIPTS = mcast4-queryfld01 mcast4-queryfld02 mcast4-queryfld03 \
	  mcast4-queryfld04 mcast4-queryfld05 mcast4-queryfld06 \
	  mcast6-queryfld01 mcast6-queryfld02 mcast6-queryfld03 \
	  mcast6-queryfld04 mcast6-queryfld05 mcast6-queryfld06

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
