#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "contrib.h"

ssize_t readline(int fd, void *str, size_t maxlen) {

	fflush(stderr);
	str = fgets((char *) str, maxlen + 1, stdin);

	if (strlen(str) > maxlen) {
		fprintf(stderr, "Message is too long, only a part will be sent");
		fflush(stderr);
	}
	return strlen(str);
}
