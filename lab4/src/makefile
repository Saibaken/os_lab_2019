process_memory:
	gcc -o process_memory process_memory.c
parallel_sum: utils.o utils.h
	gcc -o parallel_sum parallel_sum.c utils.o
utils.o: utils.h
	gcc -o utils.o -c utils.c
clean:
	rm process_memory parallel_sum utils.o