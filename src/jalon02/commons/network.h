/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#ifndef SERVER_H_SDFGEQHAEHZEGFR
#define SERVER_H_SDFGEQHAEHZEGFR

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdbool.h>

#define MAXLEN 256
#define BACKLOG 20
#define MAX_CHANS 20

/**
 * display an error with perror then quit
 */
void error(const char*);

/**
 * initialize a new ipv6 socket, controlling for error
 * @returns: the file descriptor corresponding to the new server socket
 */
int do_socket();

/**
 * initialize the server address
 */
void init_serv_addr(const char* port, struct sockaddr_in *serv_addr);

/**
 * bind the file descriptor to the configured server
 */
void do_bind(int socket, const struct sockaddr_in *serv_addr,
		socklen_t addrlen);

/**
 * accept connections on the supplied file descriptor, filling up the sockaddr structure and the client_fd
 */
int do_accept(const int socket, struct sockaddr * address, socklen_t addrlen);

/**
 * Connect the provided socket to the supplied address
 */
void do_connect(int socket, const struct sockaddr * address, socklen_t addrlen);

/**
 * read a chuck of max 256 chars and return the count of char actually read
 * @returns the number of char actually read
 */
int do_read(int socket, char * buf);

/**
 * write a string to this socket
 * @returns the number of char actually written
 */
int do_write(const int socket, const char * buf);

void do_listen(int socket);

// Converts a network endian ip address into a string to be displayed
char * iptoa(in_addr_t ip);

#endif //SERVER_H_SDFGEQHAEHZEGFR
