#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include "network.h"
#include "client.h"

#define MAXLEN 256

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

void handle_client_message(const int socket, const char * str) {
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

void handle_server_message(const int socket, char * str) {
	int res;
	memset(str, 0, MAXLEN);
	res = read(socket, str, 256);
	fprintf(stderr, "%s", str);
	fflush(stderr);

	// If the server keeps sending messages composed by 0 characters in the socket that means it has disconected the client
	if (res == 0) {
		shutdown(socket, SHUT_RDWR);
		close(socket);
		fprintf(stderr, "Connection shut down by server\n");
		fflush(stderr);
		exit(0);
	}
}
/*
 void handle_file_decline(const char* addr, const char* port) {

 }

 void handle_file_receive(const char *file_name, const char* addr,
 const char* port) {

 }

 void handle_send_file(const int socket, const char* file_path) {

 }
 */
