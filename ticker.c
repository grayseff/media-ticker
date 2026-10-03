#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>


#define MAXLINE 1024
#define WIDTH 30

int main(void)
{
	char buf[MAXLINE];
	size_t len;
	int status;

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

	if (len <= WIDTH ) {
		fputs(buf, stdout);
		putchar('\n');
		status = poll(&pfd , 1, -1);

		if (status < 0)
			return 1;
	
		goto next;
	}

	char display[WIDTH+1];
	char scroll[len * 2 + 4];
	memcpy(scroll, buf, len);
	scroll[len] = ' ';
	scroll[len+1] = ' ';
	scroll[len+2] = ' ';
	memcpy(scroll + len + 3, buf, len);
	scroll[2*len +3] = '\0';
	

	size_t i = 0;
	while (1) {
		memcpy(display, scroll + i, WIDTH);
		display[WIDTH] = '\0';
		fputs(display, stdout);
		putchar('\n');
		
		status = poll(&pfd, 1, 1000);
		if (status == 0) {
			i++;
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
