// Build with -lncurses option

#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "type.h"
#include "gameUpdate.h"
#include "gameView.h"
#include "clientTCP.h"
#include "formatage.h"
#include "clientUDP.h"
#include "chat.h"

#include <errno.h>

struct chatRecv{
  int *sockTCP;
  bool *partie_en_cours;
  Chat * chat;
  int equipe;
}chatRecv;

pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

/* Fonction du thread qui gére la reception des messages 
envoyé par les joueurs et la reception de la fin de partie (TCP)*/
void* thread_chat(void* arg){
  struct chatRecv *data = (struct chatRecv *)arg;
  int* sock = data->sockTCP;
  while (1) {
      pthread_mutex_lock(&verrou);
      if(!(*(data->partie_en_cours))){
            pthread_mutex_unlock(&verrou);
          break;
      }
      pthread_mutex_unlock(&verrou);
      int recu = 0;
      char* entete=malloc(3*sizeof(char));
      assert(entete!=NULL);
      memset(entete,0,3);
      while(recu<2){//Reception de l'entete
        int rcv=recv(*sock,entete+recu,2-recu,MSG_DONTWAIT);
        if (rcv < 0) {
          if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    // il n'y a rien a lire
                    break;
          }
          perror("erreur recv tchat ");
          continue;
        }
        
        recu+=rcv;
      }

      //On n'a rien lu on refait un tour de boucle
      if(recu==0){
        free(entete);
        continue;
      }
      recu=0;
      
      int * enteteDeform=deformat_entete(entete);
      //Reception d'un message de joueur
      if(enteteDeform[0]==13||enteteDeform[0]==14){ 
        recv(*sock,entete+2,1,0);
        int* enteteChat = NULL;
        enteteChat = deformat_entete_tchat(entete);
        assert(enteteChat!=NULL);
        int len=enteteChat[3]; //Longeur du message recu 
        char* message=malloc(len*sizeof(char));
        assert(message!=NULL);
        while(recu<len){
          int rcv=recv(*sock,message+recu,len-recu,0);
          if (rcv < 0) {
                 free(message);
                free(entete);
                free(enteteDeform);
                free(enteteChat);
                  perror("erreur recv tchat ");
                  continue;
          }       
          recu+=rcv;
        }
        
        char* buf=malloc(len+11);
        assert(buf!=NULL);
        memset(buf,0,len+11);
        
        snprintf(buf,len+11,"Joueur %d : %s",enteteChat[1],message);
        pthread_mutex_lock(&verrou);
        //On ajoute le message au chat
        add_message(data->chat,buf);

        pthread_mutex_unlock(&verrou);
        free(message);
        free(entete);
        free(buf);
        free(enteteDeform);
        free(enteteChat);
      }
      //Reception fin de partie mode solo
      else if(enteteDeform[0]==15){
        pthread_mutex_lock(&verrou);
        *(data->partie_en_cours)=false;
        
        afficher_joueurGagnant(enteteDeform[1]);
        pthread_mutex_unlock(&verrou);
        free(enteteDeform);
        free(entete);
      }
      //Reception fin de partie mode equipe
      else if(enteteDeform[0]==16){
        pthread_mutex_lock(&verrou);
        *(data->partie_en_cours)=false;
        
        afficher_EquipeGagnante(enteteDeform[2]);
        pthread_mutex_unlock(&verrou);
        free(enteteDeform);
        free(entete);
      }

  }
  free(data);
  return NULL;
}

int main(int argc, char* argv[])
{   
    if(argc < 4) {
      printf("ERREUR : format attendu :\n./client [address] [port] [mode]\n");
      return 1;
    }
    char* port = argv[2];
    char* hostname = argv[1];
    int codereq = atoi(argv[3]);
    int* sock = malloc(sizeof(int));
    assert(sock!=NULL);
    //Connexion TCP
    struct adresse_mdiff addr=socket_client(port,hostname,codereq,sock);

    
    int portUDP=addr.portUDP;
    int portMdiff=addr.portmdiff;
    char* addresse=addr.adresse;
    int* entete=addr.entete;
    //Initialisation du joueur,plateau et chat
    joueur* player=malloc(sizeof(joueur));
    assert(player!=NULL);

    pos p ;
    p.x = 0; p.y = 0;
    player->posJ=p;
    player->id=entete[1];
    player->eq=entete[2];
    if(codereq==2) fprintf(stderr, "ID EQUIPE : %d", entete[2]);
    if(codereq==1) fprintf(stderr, "ID JOUEUR : %d", entete[1]);
    board* b = malloc(sizeof(board));
    bool* partie_en_cours = malloc(sizeof(bool));
    *(partie_en_cours) = true;
    line* textSend = malloc(sizeof(line));
    assert(textSend!=NULL);
    textSend->cursor = 0;
    memset(textSend->data,0,TEXT_SIZE);
    
    Chat* chat=malloc(sizeof(Chat));
    assert(chat!=NULL);
    init_chat(chat);
    pthread_t recvChat_thread;
    struct chatRecv* allChat=malloc(sizeof(struct chatRecv));
    assert(allChat!=NULL);
    allChat->sockTCP=sock;
    allChat->partie_en_cours=partie_en_cours;
    allChat->chat=chat;
    allChat->equipe=player->eq;

    //On lance le thread de la reception TCP
    if (pthread_create(&recvChat_thread, NULL, thread_chat, allChat) != 0) {
        perror("Thread creation failed");
        close(*sock);
        free(b);
        free(partie_en_cours);
        free(textSend);
        free(chat);
        free(allChat);
        exit(EXIT_FAILURE);
    }
  
    int* sockMult=malloc(sizeof(int));
    assert(sockMult!=NULL);
    //Initialisation connexion multicast
    struct sockaddr_in6 client_addr=connexion_multicast(portMdiff,addresse,sockMult);

    // Envoi des données connexion reussi
    char* buf = format_entete(codereq+2, player->id,player->eq);
    int envoi = 0;
    while(envoi<2) {
      
      int env = send(*sock, buf+envoi, 2-envoi, 0);
      
      if(env == -1) {
        perror("Erreur send");
        close(*sock);
        free(sock);
        free(buf);
        exit(1);
      }
      envoi+=env;
    }
    free(buf);
    //on attend de recevoir la première grille complète

    setup_board(b);
    int size = 6+(b->w)*(b->h);
    char bufMessage[size];

    while(true){
      memset(bufMessage, 0, size);
      
      int recv=recvfrom(*sockMult,bufMessage,size,0,NULL,NULL);
      if(recv<0){
        
        perror("recv");
        exit(1);
      }
      
      int * enteteGrid=deformat_entete(bufMessage);
      int codereqGrid=enteteGrid[0];
      if(codereqGrid==11){
        free(enteteGrid);
        break;
      }
      free(enteteGrid);
    }
    int num_requete=0;
    
    //Initialisation de la première grille recu
    deformat_fullgrid_2(bufMessage, b);
    pthread_mutex_lock(&verrou);
    init_View();
    pthread_mutex_unlock(&verrou);
    
    //Initialisation connexion udp
    connexion_udp(*sock,portUDP,addresse,player,textSend,codereq,partie_en_cours,&verrou,chat);
  
    pthread_mutex_lock(&verrou);
    //On raffraichit l'affichage
    refresh_game_affichage(b, textSend,chat,atoi(argv[3]));
    pthread_mutex_unlock(&verrou);
    socklen_t addr_len = sizeof(client_addr);

    //mise a jour de la grille
    while (true) {
      pthread_mutex_lock(&verrou);
        if(!(*(partie_en_cours))){
          pthread_mutex_unlock(&verrou);
          break;
      }
      pthread_mutex_unlock(&verrou);
        memset(bufMessage, 0, size);
        int len = recvfrom(*sockMult, bufMessage, size, MSG_DONTWAIT , (struct sockaddr*)&client_addr, &addr_len);
        if (len < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    // il n'y a rien a lire
                    continue;
            }
            //perror("recvfrom failed");
            continue;
        }
        
        int* entete = deformat_entete(bufMessage);
        if(entete[0]==11) {//Reception de la  grille complète 
          
          int* tmp2 = deformat_fullgrid_1(bufMessage);
          int tmp = tmp2[0];

          /* On verifie le numero de requete afin de 
          savoir si on doit ignore ou non le message */
          if(tmp>num_requete || (num_requete > 60000 && tmp < 1000)) {
            num_requete = tmp;
            deformat_fullgrid_2(bufMessage,b);
          }
          free(tmp2);

        }

        //Reception de la grille partial
        else if(entete[0]==12) {

          int* tmp2 = deformat_partialgrid_1(bufMessage);
          int tmp = tmp2[0];
          int nb_cases = tmp2[1];
          free(tmp2);

          if(tmp>num_requete || (num_requete > 60000 && tmp < 1000)) {
            num_requete = tmp;
            deformat_partialgrid_2(bufMessage,b,nb_cases);
          }
        }
        free(entete);

          pthread_mutex_lock(&verrou);
          refresh_game_affichage(b, textSend,chat,atoi(argv[3]));
          pthread_mutex_unlock(&verrou);
    }
    
    sleep(7);
    free_board(b);
    pthread_mutex_lock(&verrou);
    curs_set(1); // curseur visible
    endwin(); 
    pthread_mutex_unlock(&verrou);
  
    free(player); 
    free(textSend);
    free(b);
    free(chat);
    close(*sock);
    free(sock);
    free(partie_en_cours);
    close(*sockMult);
    free(sockMult);
    fprintf(stderr, "Fin de la partie\n");
    return 0;
}

