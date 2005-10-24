SCRIPTS = tcp4-multi-sameport01 \
	  tcp4-multi-sameport02 tcp4-multi-sameport03 \
	  tcp4-multi-sameport04 tcp4-multi-sameport05 \
	  tcp4-multi-sameport06 tcp4-multi-sameport07 \
	  tcp4-multi-sameport08 \
	  tcp4-multi-sameport09 tcp4-multi-sameport10 \
	  tcp4-multi-sameport11 tcp4-multi-sameport12 \
	  tcp4-multi-sameport13 tcp4-multi-sameport14 \
	  tcp6-multi-sameport01 \
	  tcp6-multi-sameport02 tcp6-multi-sameport03 \
	  tcp6-multi-sameport04 tcp6-multi-sameport05 \
	  tcp6-multi-sameport06 tcp6-multi-sameport07 \
	  tcp6-multi-sameport08 \
	  tcp6-multi-sameport09 tcp6-multi-sameport10 \
	  tcp6-multi-sameport11 tcp6-multi-sameport12 \
	  tcp6-multi-sameport13 tcp6-multi-sameport14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
