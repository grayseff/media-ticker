ticker: ticker.c
	cproc -static -o $target $prereq
	strip $target
install: ticker
	cp ticker /home/grayseff/.local/bin/ticker

clean:
	rm -f ticker
