/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <sys/select.h>
#include <resolv.h>
#include <string.h>
#include <sys/select.h>
#include "../commons/network.h"
#include "server.h"
#include "client.h"
#include "../commons/contrib.h"

#define MAXLEN 256

int main(int argc, char** argv) {
	int quit = 0;
	int cur_fd;

	if (argc != 3) {
		fprintf(stderr, "usage: RE216_CLIENT hostname port\n");
		return 1;
	}

//get address info from the server
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

	// Initialize the fd_set
	fd_set readfd;
	fd_set fdset;

	FD_ZERO(&readfd);

	FD_ZERO(&fdset);
	FD_SET(STDIN_FILENO, &fdset);
	FD_SET(s, &fdset);

	int max_fd = s + 1;

	for (;;) {

		readfd = fdset;

		int ret = select(max_fd, &readfd, NULL, NULL, NULL);

		if (ret == -1) {
			error("Error select : ");
		}

		for (cur_fd = 0; cur_fd < max_fd && ret > 0; cur_fd++) {

			if (FD_ISSET(cur_fd, &readfd)) {
				if (cur_fd == s) {
					// Receive an echo and display it
					handle_server_message(s, buffer);
				} else if (cur_fd == STDIN_FILENO) {
					//get user input
					readline(0, buffer, MAXLEN);

					//send message to the server
					handle_client_message(s, buffer);

					// If who was sent prepare client to recieve answer
					if (strncmp(buffer, "/whois", 6) != 0 && strncmp(buffer,"/who",4) == 0) {
						puts("==================== Logged on users ====================");
					}
					// Test if log off signal was sent, if so disconnect the client
					if (strcmp(buffer, "/quit\n") == 0) {
						fprintf(stderr, "Quit signal sent\n");
						fflush(stderr);
						free(buffer);
						shutdown(s, SHUT_RDWR);
						close(s);
						exit(0);
					}
				}
				ret--;
			}
		}
	}

	//Clean buffer
	free(buffer);
	//clean up socket
	shutdown(s, SHUT_RDWR);
	close(s);

	return 0;
}
