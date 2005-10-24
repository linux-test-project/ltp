SCRIPTS = tcp4-multi-diffport01 \
	  tcp4-multi-diffport02 tcp4-multi-diffport03 \
	  tcp4-multi-diffport04 tcp4-multi-diffport05 \
	  tcp4-multi-diffport06 tcp4-multi-diffport07 \
	  tcp4-multi-diffport08 \
	  tcp4-multi-diffport09 tcp4-multi-diffport10 \
	  tcp4-multi-diffport11 tcp4-multi-diffport12 \
	  tcp4-multi-diffport13 tcp4-multi-diffport14 \
	  tcp6-multi-diffport01 \
	  tcp6-multi-diffport02 tcp6-multi-diffport03 \
	  tcp6-multi-diffport04 tcp6-multi-diffport05 \
	  tcp6-multi-diffport06 tcp6-multi-diffport07 \
	  tcp6-multi-diffport08 \
	  tcp6-multi-diffport09 tcp6-multi-diffport10 \
	  tcp6-multi-diffport11 tcp6-multi-diffport12 \
	  tcp6-multi-diffport13 tcp6-multi-diffport14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
