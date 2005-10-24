SCRIPTS = tcp4-multi-diffip01 \
	  tcp4-multi-diffip02 tcp4-multi-diffip03 \
	  tcp4-multi-diffip04 tcp4-multi-diffip05 \
	  tcp4-multi-diffip06 tcp4-multi-diffip07 \
	  tcp4-multi-diffip08 \
	  tcp4-multi-diffip09 tcp4-multi-diffip10 \
	  tcp4-multi-diffip11 tcp4-multi-diffip12 \
	  tcp4-multi-diffip13 tcp4-multi-diffip14 \
	  tcp6-multi-diffip01 \
	  tcp6-multi-diffip02 tcp6-multi-diffip03 \
	  tcp6-multi-diffip04 tcp6-multi-diffip05 \
	  tcp6-multi-diffip06 tcp6-multi-diffip07 \
	  tcp6-multi-diffip08 \
	  tcp6-multi-diffip09 tcp6-multi-diffip10 \
	  tcp6-multi-diffip11 tcp6-multi-diffip12 \
	  tcp6-multi-diffip13 tcp6-multi-diffip14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
