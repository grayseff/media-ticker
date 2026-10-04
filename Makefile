CC = cproc

ticker: ticker.c
	$(CC) -static ticker.c -o ticker

install: ticker
	strip ticker
	cp ticker $(HOME)/.local/bin/ticker

clean:
	rm -f ticker


.PHONY: install clean
