#ifndef THREADS_SERVEUR_H
#define THREADS_SERVEUR_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include "board.h"
#include "gameUpdate.h"
#include "gameView.h"   
#include <time.h>
#include "formatage.h"

#define PORT 4242
#define ADDR "localhost"
#define PREF_MULTICAST_ADDR "ff12::"
#define FREQ 20


struct tcp_thread_args {
    int* partie;
    int id_joueur;
    bool *partie_en_cours;
    pthread_mutex_t* mutex;
    pthread_mutex_t* mutex_tchat;
    int* nb_fini;
    joueur* joueurs;
    bool is_equipe;
};

struct game_args{
    board* plateau;
    int socket_mdiff;
    joueur* joueurs;
    struct sockaddr_in6 addr_mdiff;
    bool* inGame;
    pthread_mutex_t* mutex;
    action* actions;
    bool is_partie_equipe;
};

struct recv_args{
    action* actions;
    int sock_udp;
    joueur* joueurs;
    pthread_mutex_t* mutex;
    bool* partie_en_cours;
};

struct start_game_args{
    int* partie;
    int sock_udp;
    int sock_mdiff;
    struct sockaddr_in6 sock_adresse_mdiff;
    bool is_equipe;
};

case_t* changements(board* old, board* new, int* nb);
void* start_game(void* args);
void* action_request(void* args);
void* communication_tcp(void *args);
void* game_serv(void* args);

#endif // THREADS_SERVEUR_H