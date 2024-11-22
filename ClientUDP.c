#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include "gameUpdate.h"
#include "type.h"
#include "formatage.h"
#include "clientUDP.h"
#include "chat.h"

/* fonction de thread qui sert à envoyer
    les messages et les demandes d'action */
void *send_data(void *arg) {
    dataSend *data = (dataSend *)arg;
    int sock = data->sock_send;
    struct sockaddr_in6 server_addr = data->server_addr;
    memset(data->text->data,0,TEXT_SIZE);
    joueur* j = data->j;
    int codereq = (data->is_equipe) ? 6 : 5;
    
    // tant que la partie est en cours
    while (1) {

        pthread_mutex_lock(data->verrou);
        if(!(*(data->partie_en_cours))){
            pthread_mutex_unlock(data->verrou);
            break;
        }
        pthread_mutex_unlock(data->verrou);

        pthread_mutex_lock(data->verrou);
        int a = control(data->text);
        pthread_mutex_unlock(data->verrou);

        // la demande d'action n'est pas reconnue
        if (a == -1) continue;

        // chat
        if(a==6&&data->text->cursor>0){
            int envoie=0;
            int codereq_mess;
            // codereq dépend du mode de jeu (solo/équipe)
            if(codereq==6) codereq_mess = 8;
            else codereq_mess = 7;
            char* bufChat=format_tchat(codereq_mess,j->id,j->eq,data->text->cursor,data->text->data);
            int size=data->text->cursor+3;
            while(envoie<size) {
                int env = send(data->sockTCP, bufChat+envoie, size-envoie, 0);
                if(env == -1) {
                    perror("Erreur send");
                    close(data->sockTCP);
                    free(data->partie_en_cours);
                    free(data->j);
                    free(bufChat);
                    close(sock);
                    free(data);
                    exit(1);
                }
                envoie+=env;
            }

            free(bufChat);
            pthread_mutex_lock(data->verrou);
            add_message(data->chat,data->text->data);
            memset(data->text->data,0,TEXT_SIZE);
            data->text->cursor=0;
            
            delete_input_line(data->text);            
            
            
            data->text->cursor=0;
            pthread_mutex_unlock(data->verrou);
        }

        // demande d'action déplacement/bombe/annulation
        else{
            
            char* message = format_action(codereq, j->id, j->eq, data->nb_requete, a);
            int sent = sendto(sock, message, 4, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
            if (sent < 0) {
                perror("sendto failed");
                free(message);
                break;
            } 

            data->nb_requete = (data->nb_requete + 1) % 65536;
            free(message);
        }
    }
    close(sock);
    free(data);
    return NULL;
}

// crée la socket udp et lance le thread d'envoie de requetes
void connexion_udp(int sockTCP,int port,char* addr,joueur* j,line * text, int codereq, bool* partie_en_cours, pthread_mutex_t* verrou,Chat* chat){
    
    int sockSend = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockSend < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 local_addr1;
    memset(&local_addr1, 0, sizeof(local_addr1));
    local_addr1.sin6_family = AF_INET6;
    local_addr1.sin6_addr = in6addr_any;
    local_addr1.sin6_port = htons(port);
    inet_pton(AF_INET6, addr, &local_addr1.sin6_addr);

    dataSend *dataS = malloc(sizeof(dataSend));
    assert(dataS!=NULL);
    dataS->j = j;
    dataS->sock_send = sockSend;
    dataS->server_addr = local_addr1;
    dataS->is_equipe = (codereq==2);
    dataS->nb_requete = 0;
    dataS->partie_en_cours = partie_en_cours;
    dataS->verrou = verrou;
    dataS->text=text;
    dataS->sockTCP=sockTCP;
    dataS->chat=chat;

    pthread_t send_thread;
    
    if (pthread_create(&send_thread, NULL, send_data, dataS) != 0) {
        perror("Thread creation failed");
        close(sockSend);
        free(dataS);
        exit(EXIT_FAILURE);
    }

}
//s'abonne à l'adresse de multidiffusion
struct sockaddr_in6 connexion_multicast(int port,char* addr,int * sock){
    *sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (*sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int ok = 1;
    if(setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0) {
        perror("echec de SO_REUSEADDR");
        close(*sock);
        exit(1);
    }

    struct sockaddr_in6 local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin6_family = AF_INET6;
    local_addr.sin6_addr = in6addr_any;
    local_addr.sin6_port = htons(port);
    
    int r = bind(*sock, (struct sockaddr *) &local_addr, sizeof(local_addr));
    if (r < 0) {
        perror("erreur bind mdiff");
        exit(2);
    }
    struct ipv6_mreq group;
    memset(&group, 0, sizeof(group));
    inet_pton (AF_INET6, addr, &group.ipv6mr_multiaddr.s6_addr);
    group.ipv6mr_interface = if_nametoindex("eth0");
    if(setsockopt(*sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group))<0){
        perror("erreur abonnement groupe");
        close(*sock);
    }
    return local_addr;

}