SCRIPTS = tcp4-uni-basic01 \
	  tcp4-uni-basic02 tcp4-uni-basic03 \
	  tcp4-uni-basic04 tcp4-uni-basic05 \
	  tcp4-uni-basic06 tcp4-uni-basic07 \
	  tcp4-uni-basic08 \
	  tcp4-uni-basic09 tcp4-uni-basic10 \
	  tcp4-uni-basic11 tcp4-uni-basic12 \
	  tcp4-uni-basic13 tcp4-uni-basic14 \
	  tcp6-uni-basic01 \
	  tcp6-uni-basic02 tcp6-uni-basic03 \
	  tcp6-uni-basic04 tcp6-uni-basic05 \
	  tcp6-uni-basic06 tcp6-uni-basic07 \
	  tcp6-uni-basic08 \
	  tcp6-uni-basic09 tcp6-uni-basic10 \
	  tcp6-uni-basic11 tcp6-uni-basic12 \
	  tcp6-uni-basic13 tcp6-uni-basic14

all:
	@chmod +x $(SCRIPTS)

install:
	@set -e; for i in $(SCRIPTS); do ln -f $$i ../../../../bin ; done

clean:
	@set -e; for i in $(SCRIPTS); do rm -f ../../../../bin/$$i ; done
