UTILS=boxmuler.o
EXECS=cr ra radc 
SCRIPTS=modaltr.sh test.sh test2.sh
CFLAGS=-O3 
all: ${EXECS}
	@echo done

cr: create-files.o ${UTILS}
	gcc ${UTILS} create-files.o -lm -o cr

ra: random-access.o
	gcc random-access.o -o ra

radc: random-del-create.o ${UTILS}
	gcc  ${UTILS} random-del-create.o -lm -o radc

install:
	 @set -e; for i in $(EXECS) $(SCRIPTS); do ln -f $$i ../../../bin/$$i ; done

dist:
	(make clean ; cd .. ; tar zcvf fs-bench.tar.gz  fs-bench) 

clean: 
	rm ${EXECS} *.o 

