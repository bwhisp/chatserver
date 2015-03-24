/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../commons/network.h"
#include "server.h"
#include "user.h"

#include <time.h>

void main(int argc, char** argv) {

	int new_sock, cur_sock;
	int max_fd;
	int compteur = 0;
	int yes = 1;
	char buffer[MAXLEN];
	memset(buffer, 0, MAXLEN);

	//Message that will be sent to all the rejected clients
	char * refusal =
			"[Server] : Server cannot accept incoming connections anymore. Try again later. \n";

	if (argc != 2) {
		fprintf(stderr, "please specify a local port to bind to\n");
		exit(-1);
	}

	//init a fresh socket
	int s = do_socket();
	max_fd = s + 1;
	puts("% > Socket created");

	//init server address structure
	struct sockaddr_in * addr = malloc(sizeof(*addr));
	init_serv_addr(argv[1], addr);
	puts("% > Address of the server initialized");

	//init users table
	puser users = malloc(BACKLOG * sizeof(struct user));
	users_init(users);
	puts("% > Users initialized");

	//init channels table
	pchan chans = malloc(MAX_CHANS * sizeof(struct chan));
	puts("% > Channels initialized");
	chans_init(chans);

	//perform the binding
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // To avoid a pesky address already in use error
	do_bind(s, addr, sizeof(*addr));
	puts("% > Server bound");

	//specify the socket to be a server socket and listen for at most 20 concurrent client
	do_listen(s);
	puts("% > The server is listening");
	struct sockaddr_in * host = malloc(sizeof(*host));
	socklen_t hostlen = (socklen_t) sizeof(*host);

	//init the fdset
	fd_set readfd;
	fd_set fdset;

	FD_ZERO(&readfd);

	FD_ZERO(&fdset);
	FD_SET(STDIN_FILENO, &fdset);
	FD_SET(s, &fdset);

	//in an infinite loop, listen to the stored sockets

	for (;;) {
		// Restore the original fd set by copying a "static" set containing all the fd we need.
		readfd = fdset;

		//use select to listen to sockets
		int sel = select(max_fd, &readfd, NULL, NULL, NULL);

		if (sel == -1) {
			error("Error select : ");
		}

		for (cur_sock = 0; cur_sock < max_fd && sel > 0; cur_sock++) {

			if (FD_ISSET(cur_sock, &readfd)) {
				/*
				 * Incoming sockets : (1. Accept (2.Store user (3.Store socket in fd set
				 */
				//implement the listening for incoming sockets
				if (cur_sock == s) {
					//accept connection from client
					new_sock = do_accept(s, (struct sockaddr *) host, hostlen);
					compteur++;

					if (compteur > BACKLOG) {
						puts("% > Refusal");
						do_write(new_sock, refusal);
						shutdown(new_sock, SHUT_RDWR);
						close(new_sock);
						compteur--;
						break;
					}

					puts("% > Connection accepted");

					tuser client;
					client.sock_nb = new_sock;
					client.nick = get_nick(users, new_sock, buffer); //nickbuf;
					client.timez = time(NULL);
					client.port = atoi(argv[1]);
					client.ip = host->sin_addr.s_addr;
					client.chan = -1;
					add_user(users, client);

					sprintf(buffer, "[Server] : %s is connected\n",
							client.nick);
					full_bc(users, buffer);

					FD_SET(new_sock, &fdset);
					if (new_sock >= max_fd)
						max_fd = new_sock + 1;
				}
				/*
				 * Incoming messages or commands : /nick /who /whois /quit /msg /msgall
				 */
				// then implement the listening for already connected socket that write data to the server
				else {
					memset(buffer, 0, MAXLEN);
					int res = do_read(cur_sock, buffer);

					// Quit signal
					if (strncmp(buffer, "/quit", 5) == 0 || res == 0) {
						client_quit(cur_sock, &fdset, &max_fd);
						delete_user(users, cur_sock, buffer);
						compteur--;
						break;
					} // /nick command
					else if (strncmp(buffer, "/nick ", 6) == 0) {
						change_nick(users, cur_sock, buffer);
					} // /whois command
					else if (strncmp(buffer, "/whois ", 7) == 0) {
						handle_whois(users, cur_sock, buffer);
					} // /who command
					else if (strncmp(buffer, "/whois", 6) != 0
							&& strncmp(buffer, "/who", 4) == 0) {
						handle_who(users, cur_sock);
					} // /msgall command
					else if (strncmp(buffer, "/msgall ", 8) == 0) {
						send_bc(users, buffer, cur_sock);
					} // /msg command
					else if (strncmp(buffer, "/msgall ", 8) != 0
							&& strncmp(buffer, "/msg ", 5) == 0) {
						send_user(users, buffer, cur_sock);
					} // /create command
					else if (strncmp(buffer, "/create ", 8) == 0) {
						add_chan(chans, buffer, cur_sock);
						full_bc(users, buffer);
					} // /join command
					else if (strncmp(buffer, "/join ", 6) == 0) {
						join_chan(chans, users, cur_sock, buffer);
					} // /leave command to leave a channel so that we still can directly leave the chat by /quit
					else if (strncmp(buffer, "/leave", 6) == 0) {
						leave_chan(chans, users, cur_sock, buffer);
					}
					// channel multicast
					else {
						send_chan(users, chans, buffer, cur_sock, 1);
					}
				}

				sel--;
			}

		}

	} //fin for infini

	//clean up server socket
	close(s);
	free(addr);
	free(host);
}
