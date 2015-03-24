/////////////////////////////////////////////////////////////////////////////////////////
//BORDEAUX INP ENSEIRB-MATMECA
//DEPARTEMENT TELECOM
//RE216 PROGRAMMATION RESEAUX
//{daniel.negru,joachim.bruneau_-_queyreix,nicolas.herbaut}@ipb.fr
////////////////////////////////////////////////////////////////////////////////////////

#ifndef USER_H_
#define USER_H_

#include <stddef.h>
#include <netinet/in.h>

#include "network.h"

//that's the struture you should use to store your users
struct user {
	in_addr_t ip;
	int port;
	char * nick;
	int sock_nb;
	time_t timez;
	int chan;
};

typedef struct user tuser;
typedef struct user* puser;

// Structure used for channels storage
struct chan {
	int active;
	char * name;
	int members[BACKLOG];
};

typedef struct chan tchan;
typedef struct chan* pchan;

/*
 * Users handling functions
 */
void add_user(puser users, struct user client);
int search_fd(puser users, const int fd);
int search_name(puser users, const char * nick);
void change_nick(puser users, int socket, char * buffer);
void users_init(puser users);
void delete_user(puser users, int socket, char * buffer);


/*
 * Channels handling functions
 */
void add_chan(pchan chans, char * name, int sock);
void chans_init(pchan chans);
void join_chan (pchan chans, puser users, int fd, char * buffer);
int search_chan (pchan chans, char * name);
void leave_chan (pchan chans, puser users, int fd, char * buffer);
void del_chan (pchan chans, int k);
// Get the number of users currently in a given channel
int nb_users_chan (pchan chans, int id);

#endif /* USER_H_ */
