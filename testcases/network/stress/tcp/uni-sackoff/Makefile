SCRIPTS = tcp4-uni-sackoff01 \
	  tcp4-uni-sackoff02 tcp4-uni-sackoff03 \
	  tcp4-uni-sackoff04 tcp4-uni-sackoff05 \
	  tcp4-uni-sackoff06 tcp4-uni-sackoff07 \
	  tcp4-uni-sackoff08 \
	  tcp4-uni-sackoff09 tcp4-uni-sackoff10 \
	  tcp4-uni-sackoff11 tcp4-uni-sackoff12 \
	  tcp4-uni-sackoff13 tcp4-uni-sackoff14 \
	  tcp6-uni-sackoff01 \
	  tcp6-uni-sackoff02 tcp6-uni-sackoff03 \
	  tcp6-uni-sackoff04 tcp6-uni-sackoff05 \
	  tcp6-uni-sackoff06 tcp6-uni-sackoff07 \
	  tcp6-uni-sackoff08 \
	  tcp6-uni-sackoff09 tcp6-uni-sackoff10 \
	  tcp6-uni-sackoff11 tcp6-uni-sackoff12 \
	  tcp6-uni-sackoff13 tcp6-uni-sackoff14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
