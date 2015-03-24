#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 20
#define MAXLEN 256

void error(const char *msg);
int do_socket();
void init_serv_addr(const char* port, struct sockaddr_in *serv_addr);
void do_bind(int socket, const struct sockaddr_in *serv_addr, socklen_t addrlen);
void do_listen(int socket);
int do_accept(int socket, struct sockaddr * address, socklen_t addrlen);
void do_read(int socket, char * buf);

int main(int argc, char** argv) {
	int quit = 0;

	if (argc != 2) {
		fprintf(stderr, "usage: RE216_SERVER port\n");
		return 1;
	}

	//create the socket, check for validity!
	int s = do_socket();

	//init the serv_add structure
	struct sockaddr_in * addr = malloc(sizeof(*addr));
	init_serv_addr(argv[1], addr);

	//perform the binding(struct sockaddr *)
	//we bind on the tcp port specified
	do_bind(s, addr, sizeof(*addr));

	//specify the socket to be a server socket and listen for at most 20 concurrent client
	do_listen(s);

	struct sockaddr_in * host;
	socklen_t hostlen = (socklen_t) sizeof(*host);

	//accept connection from client
	int new_sock = do_accept(s, (struct sockaddr *) host, hostlen);

	for (;;) {
		char * buffer = malloc(MAXLEN);
		memset(buffer, 0, MAXLEN);
		//read what the client has to say
		do_read(new_sock, buffer);

		if (strcmp(buffer, "/quit\n") == 0) {
			fprintf(stderr, "Quit signal from client\n");
			break;
		}

		//we write back to the client
		do_write(new_sock, buffer);
		//Clean buffer
		free(buffer);
	}

	//clean up client socket
	shutdown(new_sock, SHUT_RDWR);
	close(new_sock);

	//clean up server socket
	shutdown(s, SHUT_RDWR);
	close(s);

	// Clean memory
	free(addr);

	return 0;
}

void error(const char *msg) {
	perror(msg);
	exit(1);
}

int do_socket() {
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock == -1) {
		error("Erreur serveur socket : ");
	}

	return sock;
}

void init_serv_addr(const char* port, struct sockaddr_in *serv_addr) {

	int portno;

//clean the serv_add structure
	memset(serv_addr, 0, sizeof(*serv_addr));

//cast the port from a string to an int
	portno = atoi(port);

//internet family protocol
	serv_addr->sin_family = AF_INET;

//we bind to any ip from the host
	serv_addr->sin_addr.s_addr = htonl(INADDR_ANY);

//we bind on the tcp port specified
	serv_addr->sin_port = htons(portno);

}

void do_bind(int socket, const struct sockaddr_in *serv_addr, socklen_t addrlen) {
	int res = bind(socket, (struct sockaddr *) serv_addr, addrlen);

	if (res != 0) {
		error("Erreur serveur bind : ");
	}
}

void do_listen(int socket) {
	int res = listen(socket, BACKLOG);

	if (res != 0) {
		error("Erreur serveur listen : ");
	}

}

int do_accept(int socket, struct sockaddr * address, socklen_t addrlen) {
	int new_socket = accept(socket, address, &addrlen);

	if (new_socket == -1) {
		error("Erreur serveur accept : ");
	}

	return new_socket;
}

void do_read(int socket, char * buf) {
	read(socket, buf, MAXLEN);
}

int do_write(const int socket, const char * buf) {
	ssize_t res = write(socket, buf, strlen(buf));

	if (res != strlen(buf)) {
		fprintf(stderr, "Might not be sent succesfully");
		return 1;
	}

	return 0;
}
