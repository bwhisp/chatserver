/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "user.h"

void add_user(puser users, struct user client) {
	int k = 0;
	while (users[k].sock_nb != -1 && k < BACKLOG) {
		k++;
	}
	users[k] = client;
}

void add_chan(pchan chans, char * buffer, int sock) {
	int k = 0;
	char * name = malloc(MAXLEN);
	memset(name, 0, MAXLEN);

	// Extract the expected name from the buffer and store it into name string
	sscanf(buffer, "/create %s", name);
	memset(buffer, 0, MAXLEN);
	// Do not create the channel if the name already exists
	if (search_chan(chans, name) != -1) {
		sprintf(buffer, "[Server] : This channel (%s) already exists \n", name);
		do_write(sock, buffer);
	}
	if (search_chan(chans, name) == -1) {
		// Search for an empty cell in the chans table
		while (chans[k].active != -1 && k < MAX_CHANS) {
			k++;
		}

		// Store the new chan
		chans[k].name = name;
		chans[k].active = k;

		// And format the message to be broadcasted
		sprintf(buffer, "[Server] : A new channel, %s, has been created\n",
				name);
	}

	free(name);
}

void join_chan(pchan chans, puser users, int fd, char * buffer) {
	// Recover the expected channel's name from buffer
	char * name = malloc(MAXLEN);
	memset(name, 0, MAXLEN);
	sscanf(buffer, "/join %s", name);

	int u = search_fd(users, fd);
	int c = search_chan(chans, name);

	// Send an error message in case of wrong request
	if (c == -1) {
		sprintf(buffer, "[Server] : Channel %s not found\n", name);
		do_write(fd, buffer);
		return;
	}

	// If the desired channel exists add this member and precise inform user of channel change
	sprintf(buffer, "[Server] : Welcome to channel : %s\n", name);
	do_write(fd, buffer);
	chans[c].members[u] = fd;
	users[u].chan = c;

	// Send a notification to all chan users except the new one
	sprintf(buffer, "[Server] : %s has joined the channel\n", users[u].nick);
	send_chan(users, chans, buffer, users[u].sock_nb, 0);
}

void leave_chan(pchan chans, puser users, int fd, char * buffer) {
	char * msg = malloc(MAXLEN);
	int u = search_fd(users, fd);
	int id = users[u].chan;
	int c;

	memset(msg, 0, MAXLEN);

	if (strcmp(buffer, "/leave") == 0 || strcmp(buffer, "/leave ") == 0) {
		// Format the message to be multicasted if no leave message
		sprintf(buffer, "[Server] : %s has left the channel\n", users[u].nick);
	} else {
		// Recover the leave message
		sscanf(buffer, "/leave %250[^\n]", msg);
		// Format the message to be multicasted
		sprintf(buffer, "[Server] : %s left the channel : %s\n", users[u].nick,
				msg);
	}
	send_chan(users, chans, buffer, fd, 0);

	chans[id].members[u] = -1;
	users[u].chan = -1;

	if (nb_users_chan(chans, id) == 0) {
		sprintf(msg, "[Server] : Channel %s has been deleted\n",
				chans[id].name);
		del_chan(chans, id);
		full_bc(users, msg);
	}

	free(msg);
}

int nb_users_chan(pchan chans, int id) {
	int compt = 0;
	int k;
	for (k = 0; k < BACKLOG; k++) {
		if (chans[id].members[k] != -1)
			compt++;
	}
	return compt;
}

void delete_user(puser users, int socket, char * buffer) {
	int k = search_fd(users, socket);
	char * temp = malloc(MAXLEN);
	memset(temp, 0, MAXLEN);

	if (strcmp(buffer, "/quit") == 0 || strcmp(buffer, "/quit ") == 0) {
		sprintf(buffer, "[Server] : %s has left \n", users[k].nick);
	} else {
		// Recover the quit message
		sscanf(buffer, "/quit %250[^\n]", temp);
		// Recover the username and build the message to be broadcasted
		sprintf(buffer, "[Server] : %s has left : %s \n", users[k].nick, temp);
	}
	// -1 means there is no user
	users[k].sock_nb = -1;
	users[k].nick = "\0";
	users[k].timez = 0;
	users[k].port = 0;
	users[k].ip = 0;

	full_bc(users, buffer);

	free(temp);
}

void del_chan(pchan chans, int id) {
	int j = 0;
	chans[id].active = -1;
	chans[id].name = "\0";
	while (j < BACKLOG) {
		chans[id].members[j] = -1;
		j++;
	}
}

int search_fd(puser users, const int fd) {
	int k = 0;
	while (k < BACKLOG && fd != users[k].sock_nb)
		k++;
	if (k == BACKLOG) {
		return -1;
	}
	return k;
}

int search_name(puser users, const char * nick) {
	int k = 0;
	while (k < BACKLOG && strcmp(nick, users[k].nick))
		k++;
	if (k == BACKLOG) {
		return -1;
	}
	return k;
}

int search_chan(pchan chans, char * name) {
	int k = 0;
	while (k < MAX_CHANS && strcmp(name, chans[k].name))
		k++;
	if (k == MAX_CHANS) {
		return -1;
	}
	return k;
}

void change_nick(puser users, int socket, char * buffer) {
	char * nickbuf = malloc(MAXLEN);

	// Searches whose nickname has to be changed
	int k = search_fd(users, socket);

	sscanf(buffer, "/nick %s", nickbuf);

	if (search_name(users, nickbuf) != -1) {
		do_write(users[k].sock_nb,
				"[Server] : Nickname already taken, changes discarded\n");
	} else {
		// This one will be broadcasted in the jalon4
		sprintf(buffer, "[Server] : %s is now known as %s\n", users[k].nick,
				nickbuf);
		full_bc(users, buffer);
		free(users[k].nick);
		users[k].nick = nickbuf;
	}

}

void users_init(puser users) {
	int k = 0;
	while (k < BACKLOG) {
		users[k].sock_nb = -1;
		users[k].nick = "\0";
		k++;
	}
}

void chans_init(pchan chans) {
	int k = 0;
	int j = 0;
	while (k < MAX_CHANS) {
		chans[k].active = -1;
		chans[k].name = "\0";
		while (j < BACKLOG) {
			chans[k].members[j] = -1;
			j++;
		}
		k++;
	}
}
