CC = cc
CFLAGS = -O2 -Wall -Wextra
LDLIBS = -lpulse-simple -lpulse -lm


ticker: ticker.c
	$(CC) $(CFLAGS) ticker.c -o ticker $(LDLIBS)

install: ticker
	strip ticker
	cp ticker $(HOME)/.local/bin/ticker

clean:
	rm -f ticker


.PHONY: install clean
