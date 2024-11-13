all:
	gcc -o mergesort mergesort.c -lpthread

1:
	./mergesort 1 arq1.dat arq2.dat arq3.dat -o saida.dat

2:
	./mergesort 2 arq1.dat arq2.dat arq3.dat -o saida.dat

4:
	./mergesort 4 arq1.dat arq2.dat arq3.dat -o saida.dat

8:
	./mergesort 8 arq1.dat arq2.dat arq3.dat -o saida.dat

clean:
	rm -f mergesort