SCRIPTS = tcp4-uni-winscale01 \
	  tcp4-uni-winscale02 tcp4-uni-winscale03 \
	  tcp4-uni-winscale04 tcp4-uni-winscale05 \
	  tcp4-uni-winscale06 tcp4-uni-winscale07 \
	  tcp4-uni-winscale08 \
	  tcp4-uni-winscale09 tcp4-uni-winscale10 \
	  tcp4-uni-winscale11 tcp4-uni-winscale12 \
	  tcp4-uni-winscale13 tcp4-uni-winscale14 \
	  tcp6-uni-winscale01 \
	  tcp6-uni-winscale02 tcp6-uni-winscale03 \
	  tcp6-uni-winscale04 tcp6-uni-winscale05 \
	  tcp6-uni-winscale06 tcp6-uni-winscale07 \
	  tcp6-uni-winscale08 \
	  tcp6-uni-winscale09 tcp6-uni-winscale10 \
	  tcp6-uni-winscale11 tcp6-uni-winscale12 \
	  tcp6-uni-winscale13 tcp6-uni-winscale14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
