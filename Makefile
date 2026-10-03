CC = cproc

all: media-ticker notify-ticker

media-ticker: media-ticker.c
	$(CC) -o $@ $<

notify-ticker: notify-ticker.c
	$(CC) -o $@ $<

clean:
	rm -f media-ticker notify-ticker

.PHONY: all clean
