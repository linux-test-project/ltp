SCRIPTS = tcp4-uni-smallsend01 \
	  tcp4-uni-smallsend02 tcp4-uni-smallsend03 \
	  tcp4-uni-smallsend04 tcp4-uni-smallsend05 \
	  tcp4-uni-smallsend06 tcp4-uni-smallsend07 \
	  tcp4-uni-smallsend08 \
	  tcp4-uni-smallsend09 tcp4-uni-smallsend10 \
	  tcp4-uni-smallsend11 tcp4-uni-smallsend12 \
	  tcp4-uni-smallsend13 tcp4-uni-smallsend14 \
	  tcp6-uni-smallsend01 \
	  tcp6-uni-smallsend02 tcp6-uni-smallsend03 \
	  tcp6-uni-smallsend04 tcp6-uni-smallsend05 \
	  tcp6-uni-smallsend06 tcp6-uni-smallsend07 \
	  tcp6-uni-smallsend08 \
	  tcp6-uni-smallsend09 tcp6-uni-smallsend10 \
	  tcp6-uni-smallsend11 tcp6-uni-smallsend12 \
	  tcp6-uni-smallsend13 tcp6-uni-smallsend14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
