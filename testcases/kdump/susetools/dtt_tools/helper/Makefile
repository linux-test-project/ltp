all: memdrain setuid alarm brk

memdrain: memdrain.c
	gcc -Wall -o memdrain memdrain.c

setuid: setuid.c
	gcc -Wall -o setuid setuid.c

alarm: alarm.c
	gcc -Wall -o alarm alarm.c

brk: brk.c
	gcc -Wall -o brk brk.c

clean:
	rm -f memdrain setuid alarm brk *.o
