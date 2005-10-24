SCRIPTS = tcp4-uni-tso01 \
	  tcp4-uni-tso02 tcp4-uni-tso03 \
	  tcp4-uni-tso04 tcp4-uni-tso05 \
	  tcp4-uni-tso06 tcp4-uni-tso07 \
	  tcp4-uni-tso08 \
	  tcp4-uni-tso09 tcp4-uni-tso10 \
	  tcp4-uni-tso11 tcp4-uni-tso12 \
	  tcp4-uni-tso13 tcp4-uni-tso14 \
	  tcp6-uni-tso01 \
	  tcp6-uni-tso02 tcp6-uni-tso03 \
	  tcp6-uni-tso04 tcp6-uni-tso05 \
	  tcp6-uni-tso06 tcp6-uni-tso07 \
	  tcp6-uni-tso08 \
	  tcp6-uni-tso09 tcp6-uni-tso10 \
	  tcp6-uni-tso11 tcp6-uni-tso12 \
	  tcp6-uni-tso13 tcp6-uni-tso14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
