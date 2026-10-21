CC = cc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS = -static
LDLIBS = -lm


ticker: ticker.c
	$(CC) $(CFLAGS) ticker.c -o ticker $(LDFLAGS) $(LDLIBS)

install: ticker
	strip ticker
	mkdir -p $(HOME)/.local/bin $(HOME)/.local/share/man/man1
	cp ticker $(HOME)/.local/bin/ticker
	cp ticker.1 $(HOME)/.local/share/man/man1/ticker.1

uninstall:
	rm -f $(HOME)/.local/bin/ticker
	rm -f $(HOME)/.local/share/man/man1/ticker.1
clean:
	rm -f ticker


.PHONY: install clean
