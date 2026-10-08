#include <locale.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#define MAXLINE 1024
#define WIDTH 45
#define OUTFILE "/tmp/current-ticker"

static size_t textwidth(const char *);
static int notify(char *, size_t);
static int media(const char *, size_t, struct pollfd *);

int
main(void)
{
	char buf[MAXLINE];
	char mediabuf[MAXLINE] = " ";
	size_t width = 0;
	size_t len;
	struct pollfd pfd;

	setlocale(LC_CTYPE, "");
	pfd.fd = STDIN_FILENO;
	pfd.events = POLLIN;

	while (1) {
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			return 1;

		len = strlen(buf);
		if (len > 0 && buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
			len--;
		}

		if (buf[0] == 'N') {
			if (notify(buf, len))
				return 1;
		}

		if (buf[0] == 'M') {
			strcpy(mediabuf, buf + 2);
			width = textwidth(mediabuf);
		}

		if (media(mediabuf, width, &pfd))
			return 1;
	}

	return 0;
}

static size_t
textwidth(const char *p)
{
	size_t width = 0;
	wchar_t wc;
	mbstate_t state = {0};

	while (*p != '\0') {
		size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &state);
		int w;

		if ( n == 0 || n == (size_t)-1 || n == (size_t)-2)
			break;

		w = wcwidth(wc);
		if (w < 0)
			break;
		width += w;
		p += n;
	}

	return width;
}

static int
notify(char *buf, size_t len)
{
	const char header[] = " * NOTIFICATION * - ";
	size_t msglen = len - 2;
	size_t cycle = sizeof(header) - 1 + msglen;
	size_t width = textwidth(header) + textwidth(buf + 2);
	size_t padding = cycle < WIDTH ? WIDTH - width : 0;
	size_t period = cycle + padding + 5;
	char display[WIDTH * 4 + 1];
	char scroll[2 * period + 1];
	wchar_t wc;
	int i = 0;
	int rotations = 0;

	memcpy(scroll, header, sizeof(header) - 1);
	memcpy(scroll + sizeof(header) - 1, buf + 2, msglen);

	memset(scroll + cycle, ' ', padding + 5);

	memcpy(scroll + period, scroll, period);
	scroll[2 * period] = '\0';

	while (rotations < 2) {
		size_t bytes = 0;
		size_t cols = 0;
		size_t n0 = 0;
		FILE *out;
		mbstate_t state = {0};

		while (1) {
			int w;
			size_t n = mbrtowc(&wc, scroll + i + bytes, MB_CUR_MAX, &state);
			
			if (n == 0 || n == (size_t)-1 || n==(size_t)-2){
				break;
			}
			w = wcwidth(wc);
			if (w < 0)
				break;

			if (cols + w > WIDTH)
				break;

			if (bytes == 0)
				n0 = n;

			bytes += n;
			cols += w;
		}

		memcpy(display, scroll + i, bytes);
		display[bytes] = '\0';
		out = fopen(OUTFILE, "w");

		if (out == NULL)
			return 1;

		fputs(display, out);
		if (fclose(out) != 0)
			return 1;

		usleep(250000);
		i += n0;

		if (i >= period) {
			i = 0;
			rotations++;
		}
	}
	return 0;
}

static int
media(const char *msg, size_t width, struct pollfd *pfd)
{
	int pollstatus;
	size_t msglen = strlen(msg);
	wchar_t wc;

	if (width <= WIDTH) {
		FILE *out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;
		fputs(msg, out);
		if (fclose(out) != 0)
			return 1;
		pollstatus = poll(pfd, 1, -1);
		if (pollstatus < 0)
			return 1;
		return 0;
	}

	char display[WIDTH * 4 + 1];
	char scroll[msglen * 2 + 4];
	size_t i = 0;

	memcpy(scroll, msg, msglen);
	scroll[msglen] = ' ';
	scroll[msglen + 1] = ' ';
	scroll[msglen + 2] = ' ';
	memcpy(scroll + msglen + 3, msg, msglen);
	scroll[2 * msglen + 3] = '\0';

	while (1) {
		size_t bytes = 0;
		size_t cols = 0;
		size_t n0 = 0;
		FILE *out;
		mbstate_t state = {0};
			
		while (1) {
		int w;
		size_t n = mbrtowc(&wc, scroll + i + bytes, MB_CUR_MAX, &state);
		if (n == 0 || n == (size_t)-1 || n==(size_t)-2){
			break;
		}
		w = wcwidth(wc);
		if (w < 0)
			break;

		if (cols + w > WIDTH)
			break;
		if (bytes == 0)
			n0 = n;

			bytes += n;
			cols += w;
		}

		memcpy(display, scroll + i, bytes);
		display[bytes] = '\0';
		out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;
		fputs(display, out);

		if (fclose(out) != 0)
			return 1;
		
		pollstatus = poll(pfd, 1, 1000);
		if (pollstatus == 0) {
			i += n0;
			if (i >= msglen + 3)
				i = 0;
		}
		if (pollstatus > 0 && (pfd->revents & POLLIN))
			return 0;
		if (pollstatus < 0)
			return 1;
	}
	return 0;
}


