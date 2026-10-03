CC = cproc

ticker: ticker.o
	$(CC) ticker.o -o ticker

ticker.o: ticker.c
	$(CC) -c ticker.c

clean:
	rm -f ticker ticker.o

.PHONY: clean
