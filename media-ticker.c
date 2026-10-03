#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <locale.h>
#include <wchar.h>


#define MAXLINE 1024
#define WIDTH 30
#define OUTFILE "/tmp/current-media"

int main(void)
{

	setlocale(LC_CTYPE,"");

	char buf[MAXLINE];
	size_t len;
	int status;

	wchar_t wc;
	mbstate_t state = {0};

	
	struct pollfd pfd;
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
	
	char *p = buf;
	size_t width = 0;

	state = (mbstate_t){0};

		
	while(*p != '\0'){
		size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &state);
		int w = wcwidth(wc);
		width +=w;
		p += n;
	}
	if (width <= WIDTH ) {
		/*fputs(buf, stdout);
		putchar('\n');*/

		FILE *out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;
		fputs(buf, out);
		if (fclose(out) != 0)
			return 1;
				
		status = poll(&pfd , 1, -1);

		if (status < 0)
			return 1;
	
		goto next;
	}

	char display[WIDTH * 4 + 1];
	char scroll[len * 2 + 4];
	memcpy(scroll, buf, len);
	scroll[len] = ' ';
	scroll[len+1] = ' ';
	scroll[len+2] = ' ';
	memcpy(scroll + len + 3, buf, len);
	scroll[2*len +3] = '\0';
	

	size_t i = 0;
	while (1) {
		size_t bytes = 0;
		size_t cols = 0;
		size_t n_0 = 0;
		
		state = (mbstate_t){0};
			
		while (1) {
			size_t n = mbrtowc(&wc, scroll + i + bytes, MB_CUR_MAX, &state);
			int w = wcwidth(wc);

			if (cols + w > WIDTH)
				break;
			if (bytes == 0)
				n_0 = n;
			bytes += n;
			cols += w;
		}

		memcpy(display, scroll + i, bytes);
		display[bytes] = '\0';
		/*
		fputs(display, stdout);
		putchar('\n');
		*/

		FILE *out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;
		fputs(display, out);
		if (fclose(out) != 0)
			return 1;
		
		status = poll(&pfd, 1, 1000);
		if (status == 0) {
			
			i += n_0;
			if (i >= len + 3) {
				i = 0;
			}
		}
		if ( status > 0 && (pfd.revents & POLLIN))
			goto next;
		if (status < 0)
			return 1;
		
		

		
	}

	return 0;
}
