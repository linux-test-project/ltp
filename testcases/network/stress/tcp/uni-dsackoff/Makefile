SCRIPTS = tcp4-uni-dsackoff01 \
	  tcp4-uni-dsackoff02 tcp4-uni-dsackoff03 \
	  tcp4-uni-dsackoff04 tcp4-uni-dsackoff05 \
	  tcp4-uni-dsackoff06 tcp4-uni-dsackoff07 \
	  tcp4-uni-dsackoff08 \
	  tcp4-uni-dsackoff09 tcp4-uni-dsackoff10 \
	  tcp4-uni-dsackoff11 tcp4-uni-dsackoff12 \
	  tcp4-uni-dsackoff13 tcp4-uni-dsackoff14 \
	  tcp6-uni-dsackoff01 \
	  tcp6-uni-dsackoff02 tcp6-uni-dsackoff03 \
	  tcp6-uni-dsackoff04 tcp6-uni-dsackoff05 \
	  tcp6-uni-dsackoff06 tcp6-uni-dsackoff07 \
	  tcp6-uni-dsackoff08 \
	  tcp6-uni-dsackoff09 tcp6-uni-dsackoff10 \
	  tcp6-uni-dsackoff11 tcp6-uni-dsackoff12 \
	  tcp6-uni-dsackoff13 tcp6-uni-dsackoff14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
