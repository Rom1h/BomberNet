#ifndef CLIENTUDP_H  
#define CLIENTUDP_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
 #include <net/if.h>

#include "gameUpdate.h"
#include "type.h"
#include "formatage.h"

typedef struct {
    joueur* j;
    int sock_send;
    int sockTCP;
    int nb_requete;
    bool is_equipe;
    struct sockaddr_in6 server_addr;
    bool* partie_en_cours;
    pthread_mutex_t* verrou;
    line * text;
    Chat* chat;
} dataSend;
        
typedef struct {
    board* b;
    joueur* j;
    int sock_recv;
    struct sockaddr_in6 server_addr;
} dataRecv;

// Function Prototypes
void *send_data(void *arg);
void *receive_data(void *arg);
char* message_action(int mode, int nb_requete, joueur* j, int a);
void update_game(char* buffer, board* b);
int action_Player(int a, board* b, joueur* j);
void connexion_udp(int sockTCP,int port, char* addr, joueur* j,line * l, int codereq, bool* partie_en_cours, pthread_mutex_t *verrou,Chat* chat);
struct sockaddr_in6 connexion_multicast(int port, char* addr, int* sock);
#endif