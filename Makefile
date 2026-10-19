CC = cc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS = -static
LDLIBS = -lm


ticker: ticker.c
	$(CC) $(CFLAGS) ticker.c -o ticker $(LDFLAGS) $(LDLIBS)

install: ticker
	strip ticker
	cp ticker $(HOME)/.local/bin/ticker

clean:
	rm -f ticker


.PHONY: install clean
