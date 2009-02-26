test_base: tbase_ki.o user_tbase.o
	${CC} tbase_ki.o user_tbase.o -o test_base

tbase_ki.o: tbase_ki.c
	${CC} -c tbase_ki.c

user_tbase.o: user_tbase.c
	${CC} -c user_tbase.c

clean: 
	 rm -f *.o 2>/dev/null || true
