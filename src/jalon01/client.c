#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAXLEN 256

struct addrinfo** get_addr_info(const char* address, const char* port,
		struct addrinfo** res);
int do_socket();
void do_connect(int socket, const struct sockaddr * address, socklen_t addrlen);
ssize_t readline(int fd, void *str, size_t maxlen);
void handle_client_message(int socket, const void * str);

int main(int argc, char** argv) {
	int quit = 0;

	if (argc != 3) {
		fprintf(stderr, "usage: RE216_CLIENT hostname port\n");
		return 1;
	}

//get address info from the server
//get_addr_info()
	struct addrinfo addr;
	struct addrinfo * add = &addr;
	struct addrinfo ** ad = &add;

	ad = get_addr_info(argv[1], argv[2], ad);

	add = *ad;
	addr = *add;

//get the socket
	int s;
	s = do_socket();

//connect to remote socket
	do_connect(s, addr.ai_addr, addr.ai_addrlen);

	char * buffer = malloc(MAXLEN);
	memset(buffer, 0, MAXLEN);

	for(;;) {
		//get user input
		readline(0, buffer, MAXLEN);

		//send message to the server
		handle_client_message(s, buffer);

		// Test if log off signal was sent
		if (strcmp(buffer, "/quit\n") == 0) {
			fprintf(stderr, "Quit signal sent\n");
			break;
		}
		// Receive an echo and display it
		memset(buffer, 0, MAXLEN);
		read(s, buffer, 256);
		fprintf(stderr, "--> %s", buffer);
	}

	//Clean buffer
	free(buffer);
	//clean up socket
	shutdown(s, SHUT_RDWR);
	close(s);

	return 0;
}

struct addrinfo ** get_addr_info(const char* address, const char* port,
		struct addrinfo** res) {

	int status;
	struct addrinfo hints, p;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(address, port, &hints, res);

	if (status != 0) {
		perror("Erreur client address info : ");
	}
	return res;
}

int do_socket() {
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock == -1) {
		perror("Erreur client socket : ");
	}

	return sock;
}

void do_connect(int socket, const struct sockaddr * address, socklen_t addrlen) {
	int c;

	c = connect(socket, address, addrlen);

	if (c != 0) {
		perror("Erreur client connect : ");
	}

}

ssize_t readline(int fd, void *str, size_t maxlen) {
	fprintf(stderr, "Message (%d characters max) \n", (int) maxlen);
	str = fgets((char *) str, maxlen + 1, stdin);

	if (strlen(str) > maxlen) {
		fprintf(stderr, "Message is too long, only a part will be sent");
	}
	return strlen(str);
}

void handle_client_message(int socket, const void * str) {
	size_t len = strlen((char *) str);
	ssize_t res;
	char * buf = malloc(MAXLEN);
	memset(buf, 0, MAXLEN);

	//Message
	res = write(socket, str, len);
	if (res != len) {
		perror("Erreur ecriture message :");
	}

	free(buf);
}
