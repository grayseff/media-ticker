#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>

#define MAXLINE 1024
#define WIDTH 30
#define OUTFILE "/tmp/current-notification"

int
main(void)
{
	setlocale(LC_CTYPE, "");

	char buf[MAXLINE];
	size_t len;

	wchar_t wc;
	mbstate_t state = {0};

	next:
	if (fgets(buf, sizeof(buf), stdin) == NULL)
		return 1;

	len = strlen(buf);

	if (len > 0 && buf[len - 1] == '\n') {
		buf[len - 1] = '\0';
		len--;
	}

	char *p = buf;
	size_t width = 0;

	state = (mbstate_t){0};

	while (*p != '\0') {
		size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &state);
		int w = wcwidth(wc);

		width += w;
		p += n;
	}


	if (width <= WIDTH) {
		FILE *out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;

		fputs(buf, out);

		if (fclose(out) != 0)
			return 1;

		sleep(10);

		out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;

		if (fclose(out) != 0)
			return 1;

		goto next;
	}
	char display[WIDTH * 4 + 1];
	char scroll[len * 2 + 4];

	memcpy(scroll, buf, len);
	scroll[len] = ' ';
	scroll[len + 1] = ' ';
	scroll[len + 2] = ' ';
	memcpy(scroll + len + 3, buf, len);
	scroll[2 * len + 3] = '\0';

	size_t i = 0;
	int rotations = 0;

	while (rotations < 3) {
		size_t bytes = 0;
		size_t cols = 0;
		size_t n_0 = 0;

		state = (mbstate_t){0};

		while (1) {
			size_t n = mbrtowc(&wc, scroll + i + bytes,
			    MB_CUR_MAX, &state);
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

		FILE *out = fopen(OUTFILE, "w");
		if (out == NULL)
			return 1;

		fputs(display, out);

		if (fclose(out) != 0)
			return 1;

		usleep(200000);

		i += n_0;

		if (i >= len + 3) {
			i = 0;
			rotations++;
		}
	}

	FILE *out = fopen(OUTFILE, "w");
	if (out == NULL)
		return 1;

	if (fclose(out) != 0)
		return 1;

	goto next;



}
