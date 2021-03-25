process_memory:
	gcc -o process_memory process_memory.c
parallel_sum: utils.o utils.h sum.h sum.o
	gcc -pthread -o parallel_sum parallel_sum.c utils.o sum.o
utils.o: utils.h
	gcc -o utils.o -c utils.c
sum.o: sum.h
	gcc -o sum.o -c sum.c
libSUM.a: sum.o
	ar rc libSUM.a sum.o
	ranlib libSUM.a	

clean:
	rm process_memory parallel_sum utils.o sum.o