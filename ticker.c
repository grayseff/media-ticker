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

int 
main(void)
{
	char buf[MAXLINE];

	char media_buf[MAXLINE] = " ";
	const char header[] = "                          * NOTIFICATION * - ";
	char *p;

	size_t media_width = 0;
	size_t len;
	size_t width;
	
	int status;
	int notify = 0;

	wchar_t wc;
	mbstate_t state = {0};
	struct pollfd pfd;

	
	setlocale(LC_CTYPE,"");

	
	pfd.fd = STDIN_FILENO;
	pfd.events = POLLIN;


next:
	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return 1;

	len = strlen(buf);
	
	if (len > 0 && buf[len - 1] == '\n'){
		buf[len - 1] = '\0';
		len--;
	}
	
	p = buf;
	width = 0;

	state = (mbstate_t){0};

	if (buf[0] == 'N') {
		notify = 1;
		p += 2;
	} else if (buf[0] == 'M') {
		p += 2;
		strcpy(media_buf, p);
	} else {
		goto next;
	}

	while (*p != '\0') {
		size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &state);
		int w = wcwidth(wc);
		width += w;
		p += n;
	}
	if (!notify)
		media_width = width;

notify:		
	if (notify) {
/* make notify buffer */
		size_t msglen = len - 2;
		size_t i;
		char display[WIDTH * 4 + 1];
		char scroll[(msglen) * 2 + 91 ]; 
		int rotations;

		memcpy(scroll, header, WIDTH);
		memcpy(scroll + WIDTH, buf + 2, msglen);
		memcpy(scroll + WIDTH + msglen, header, WIDTH);
		memcpy(scroll + 90 + msglen, buf + 2, msglen);
		scroll[ 2 * msglen + 90] = '\0';
		
		i = 0;
		rotations = 0;

		while (rotations < 2) {
			size_t bytes = 0;
			size_t cols = 0;
			size_t n0 = 0;
			size_t n;
			int w;
			FILE *out;


			state = (mbstate_t){0};
			while (1) {

				n = mbrtowc(&wc, scroll + i + bytes, MB_CUR_MAX, &state);
				if (n == 0){
					break;}
				w = wcwidth(wc);
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
			if (i >= msglen + WIDTH) {
				i = 0;
				rotations++;
			}
		}
		notify = 0;
	}
media:
	/* Short Media */
	if (media_width <= WIDTH) {
		FILE *out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;
		fputs(media_buf, out);
		if (fclose(out) != 0)
			return 1;
		status = poll(&pfd, 1, -1);
		if (status < 0)
			return 1;
		goto next;
		
	}
	/* Long Media */
	size_t msglen = strlen(media_buf);
	size_t i;

	char display[WIDTH * 4 + 1];
	char scroll[msglen * 2 + 4];

	memcpy(scroll, media_buf, msglen);
	scroll[msglen] = ' ';
	scroll[msglen + 1] = ' ';
	scroll[msglen + 2] = ' ';
	memcpy(scroll + msglen + 3, media_buf, msglen);
	scroll[2 * msglen +3] = '\0';
		

	i = 0;
	while (1) {
		size_t bytes = 0;
		size_t cols = 0;
		size_t n_0 = 0;
		FILE *out;
		
		state = (mbstate_t){0};
			
		while (1) {
			size_t n = mbrtowc(&wc, scroll + i + bytes, MB_CUR_MAX, &state);
			int w;

			if (n == 0)
				break;
			w = wcwidth(wc);

			if (cols + w > WIDTH)
				break;
			if (bytes == 0)
				n_0 = n;
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
		
		status = poll(&pfd, 1, 1000);
		if (status == 0) {
			
			i += n_0;
			if (i >= msglen + 3) {
				i = 0;
			}
		}
		if (status > 0 && (pfd.revents & POLLIN))
			goto next;
		if (status < 0)
			return 1;
		
		

		
	}

	return 0;
}
