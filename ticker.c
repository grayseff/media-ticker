#include <locale.h>
#include <poll.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wchar.h>

#define MAXLINE 1024
#define WIDTH 45
#define OUTFILE "/tmp/current-ticker"

static size_t textwidth(const char *);
static void ding(void);
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
	signal(SIGPIPE, SIG_IGN);
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
			ding();
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

static void
ding(void)
{
	enum { RATE = 44100, SAMPLES = RATE / 10 };
	int16_t pcm[SAMPLES];
	int fd[2];
	const char *p;
	size_t remaining;

	for (int i = 0; i < SAMPLES; i++) {
		double t = (double)i / RATE;
		double volume = 1.0 - (double)i / SAMPLES;

		pcm[i] = 9000 * volume * (
				sin(2.0 * M_PI * 880.0 * t)
				+ 0.35 * sin(2.0 * M_PI * 1320.0 * t)
				);
	}
	if ( pipe(fd) < 0 )
		return;
	pid_t pid = fork();
	if (pid < 0) {
		close(fd[0]);
		close(fd[1]);
		return;
	}

	if (pid == 0) {
		/* Child: run paplay with the pipe as stdin. */
		close(fd[1]);
		if (dup2(fd[0], STDIN_FILENO) < 0 )
			_exit(1);
		close(fd[0]);

		execlp("paplay", "paplay",
			"--raw",
			"--rate=44100",
			"--format=s16ne",
			"--channels=1",
			(char *)NULL);
		_exit(127);
	}
	/* Parent: send the samples to paplay. */
	close(fd[0]);

	p = (const char *)pcm;
	remaining = sizeof(pcm);

	while (remaining > 0) {
		ssize_t n = write(fd[1], p, remaining);
		if (n <= 0)
			break;
		p += n;
		remaining -= n;
	}

	close(fd[1]);
	waitpid(pid, NULL, 0);
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


