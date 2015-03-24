/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#ifndef SERVER_H_QSGPNSGPOQNSPGOSNDFG
#define SERVER_H_QSGPNSGPOQNSPGOSNDFG

#include "user.h"

// Cleans memory if a client logs off
void client_quit(int socket, fd_set * fdset, int * max_fd);

// Get nickname as someone is connected
char * get_nick(puser users, int sock, char buffer[]);

// Send to all users except the sender, identified by fd
void send_bc(puser users, char * buffer, int fd);

// Send to all users
void full_bc(puser users, char * buffer);

// Send to a specific user from an other one
void send_user(puser users, char * buffer, int sender);

// Send to all users of a channel
void send_chan(puser users, pchan chans, char * buffer, int sender_fd,
		int option);

// Process /who command
void handle_who(puser users, int socket);

// Process /whois command
void handle_whois(puser users, int socket, char * buffer);


#endif //SERVER_H_QSGPNSGPOQNSPGOSNDFG
