/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <asm-generic/socket.h>
#include <regex.h>
#include <resolv.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "../commons/network.h"
#include "server.h"
#include "callbacks.h"

void client_quit(int socket, fd_set * fdset, int * max_fd) {
	int i;

	// We close the socket and we remove its file descriptor from the main fd set
	shutdown(socket, 2);
	close(socket);
	FD_CLR(socket, fdset);

	// If it was the max fd we readjust the max_fd variable
	if (socket == *max_fd + 1) {
		for (i = 0; i < *max_fd; i++) {
			if (!FD_ISSET(i, fdset))
				break;
		}
		*max_fd = i;
	}
}

char * get_nick(puser users, int sock, char buffer[]) {
	char * nickname = malloc(MAXLEN);
	char * nick_request =
			"[Server] : Please specify a nickname (/nick your_nick) \n";
	char * welcome = malloc(MAXLEN);
	memset(buffer, 0, MAXLEN);
	memset(nickname, 0, MAXLEN);

	while (1) {
		do_write(sock, nick_request);
		memset(buffer, 0, MAXLEN);
		do_read(sock, buffer);
		if (strncmp(buffer, "/nick ", 6) == 0) {
			sscanf(buffer, "/nick %s", nickname);
		}
		if (search_name(users, nickname) == -1) {
			break;
		}
		do_write(sock, "[Server] : Nickname already taken or incorrect \n");
	}

	sprintf(welcome, "[Server] : Welcome to the chat %s\n", nickname);
	do_write(sock, welcome);

	return nickname;
}

void send_bc(puser users, char * buffer, int fd) {
	int k = 0;
	int user_id = search_fd(users, fd);
	char * msg = malloc(MAXLEN);
	memset(msg, '0', MAXLEN);

	sscanf(buffer, "/msgall %250[^\n]", msg);
	sprintf(buffer, "%s : %s \n", users[user_id].nick, msg);

	for (k = 0; k < BACKLOG; k++) {
		if (users[k].sock_nb != fd && users[k].sock_nb != -1)
			do_write(users[k].sock_nb, buffer);
	}
	free(msg);
}

void full_bc(puser users, char * buffer) {
	int k = 0;

	for (k = 0; k < BACKLOG; k++) {
		if (users[k].sock_nb != -1)
			do_write(users[k].sock_nb, buffer);
	}
}

void send_chan(puser users, pchan chans, char * buffer, int sender_fd,
		int option) {
	char * message = malloc(MAXLEN);
	int k = 0;
	int fd = -1;

	// Search for the user's channel
	int chan_nb = users[search_fd(users, sender_fd)].chan;

	if (option == 1) { // Option = 1 if user sent message, and 0 if it is the server
		// Get the sender's id to have his nickname when formatting
		int u = search_fd(users, sender_fd);
		// Format the message
		sprintf(message, "{%s} : %s", users[u].nick, buffer);
	}
	else {
		sprintf(message, "%s",buffer);
	}

	// Send to all users of the channel
	while (k < BACKLOG) {
		fd = chans[chan_nb].members[k];
		if (fd != -1 && fd != sender_fd) {
			do_write(fd, message);
		}
		k++;
	}

	free(message);
}

void send_user(puser users, char * buffer, int sender) {
	char * temp = malloc(MAXLEN);

	// Extracting the expected reciever's nickname from the buffer
	sscanf(buffer, "/msg %s", temp);
	// Search for the reciever's id by its name and search for the sender's id by its socket
	int r = search_name(users, temp);
	int s = search_fd(users, sender);
	if (r == -1) {
		sprintf(buffer, "[Server] : %s is not a logged on user \n", temp);
		do_write(sender, buffer);
		return;
	}
	// Extract the message from the buffer, we use a regex to have the spaces included in the message
	sscanf(buffer, "/msg %*s %250[^\n]", temp);
	// Format the message to be sent
	sprintf(buffer, "[%s] : %s\n", users[s].nick, temp);

	// Finally send the message to the user
	do_write(users[r].sock_nb, buffer);

	//
	free(temp);
}

void handle_who(puser users, int socket) {
	char * buffer = malloc(MAXLEN);
	int compteur = 0;
	int k = 0;
	for (k = 0; k < BACKLOG; k++) {
		memset(buffer, '0', MAXLEN);
		if (users[k].sock_nb != -1) {
			compteur++;
			sprintf(buffer, "%d. ", compteur);
			strcat(buffer, users[k].nick);
			strcat(buffer, "\n");
			do_write(socket, buffer);
		}
	}
	free(buffer);
}

void handle_whois(puser users, int socket, char * buffer) {
	char * name = malloc(MAXLEN);
	char timebuf[20];
	int k;
	memset(name, '0', MAXLEN);
	sscanf(buffer, "/whois %s", name);
	k = search_name(users, name);

	if (k != -1) {
		time_t time = users[k].timez;
		strftime(timebuf, 20, "%Y/%m/%d@%H:%M:%S", localtime(&time));
		sprintf(buffer,
				"[Server] : %s is connected since %s with IP address %s and port number %i\n",
				name, timebuf, iptoa(users[k].ip), users[k].port);
	} else {
		sprintf(buffer, "[Server] : %s appears not to be a logged on user\n",
				name);
	}

	do_write(socket, buffer);
}
