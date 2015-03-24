/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <regex.h>
#include <resolv.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "user.h"
#include "callbacks.h"
#include "colors.h"
#include "network.h"

void error(const char* msg) {
	perror(msg);
	exit(-1);
}

/**************************************************
 * In this file, you should implement every code that
 * can be used by both client and server
 */

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

int do_accept(const int socket, struct sockaddr * address, socklen_t addrlen) {
	int new_socket = accept(socket, address, &addrlen);

	if (new_socket == -1) {
		error("Erreur serveur accept : ");
	}

	return new_socket;
}

int do_read(int socket, char * buf) {
	return read(socket, buf, MAXLEN);
}

int do_write(const int socket, const char * buf) {
	ssize_t res = write(socket, buf, strlen(buf));

	if (res != strlen(buf)) {
		fprintf(stderr,"Length : %i \n",(int) strlen(buf));
		fprintf(stderr,"Result : %i \n",(int) res);
		puts("Might not be sent succesfully");
	}

	return res;
}

void do_listen(int socket) {
	int res = listen(socket, BACKLOG);

	if (res != 0) {
		error("Erreur serveur listen : ");
	}

}

// I am truly forced to implement this function because gcc keeps telling me inet_ntoa returns an integer
char * iptoa(in_addr_t ip) {
	static char addr[16];
	unsigned char* champ = (unsigned char *) &ip;
	sprintf(addr, "%d.%d.%d.%d", champ[0], champ[1], champ[2], champ[3]);
	return addr;
}
