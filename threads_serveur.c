#include "threads_serveur.h"
#include <assert.h>

/* fonctions des threads lancés par le serveur */

/* renvoie les cases de la grille qui ont changé */
case_t* changements(board* old, board* new, int* nb){
    size_t taille = 0;
    for(int x = 0; x<old->w; x++){
        for(int y = 0; y<old->h; y++){
            if(get_grid(old,x,y)!=get_grid(new,x,y)){
                taille++;
            }
        }
    }
    struct case_t* first_tab = malloc(taille*sizeof(case_t));
    assert(first_tab!=NULL);
    int cpt = 0;
    for(int x = 0; x<old->w; x++){
        for(int y = 0; y<old->h; y++){
            if(get_grid(old,x,y)!=get_grid(new,x,y)){
                (*(first_tab+cpt)).position.x=x;
                (*(first_tab+cpt)).position.y=y;
                (*(first_tab+cpt)).valeur=get_grid(new,x,y);
                cpt++;
            }
        }
    }
    *(nb) = taille;
    return first_tab;
}

/* lance tous les threads nécessaires à la partie */
void* start_game(void* args){
    struct start_game_args* mes_args = (struct start_game_args*)(args);
    int* partie = mes_args->partie;
    int sock_udp = mes_args->sock_udp;
    int sock_mdiff = mes_args->sock_mdiff;
    struct sockaddr_in6 sock_adresse_mdiff=mes_args->sock_adresse_mdiff;
    bool is_equipe = mes_args->is_equipe;

    pthread_t les_threads[6];

    bool* is_enCours = malloc(sizeof(bool));
    assert(is_enCours!=NULL);
    *is_enCours = true;
    
    pthread_mutex_t *mutex;
    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    assert(mutex!=NULL);
    if(pthread_mutex_init(mutex, NULL)!=0){
        perror("Initialisation de mutex");
        exit(EXIT_FAILURE);
    }
    
    pthread_mutex_t *mutex_tchat;
    mutex_tchat = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    assert(mutex_tchat!=NULL);
    if(pthread_mutex_init(mutex_tchat, NULL)!=0) {
        perror("Initialisation de mutex_tchat");
        exit(EXIT_FAILURE);
    }
    
    struct tcp_thread_args *mes_args_tcp = malloc(sizeof(struct tcp_thread_args)*4);
    assert(mes_args_tcp!=NULL);
    
    struct board* plateau = malloc(sizeof(board));
    assert(plateau!=NULL);
    struct joueur* joueurs = setup_board(plateau);
    assert(joueurs!=NULL);

    for(size_t i = 0; i<4; ++i) {
        (mes_args_tcp+i)->partie = partie;
        (mes_args_tcp+i)->id_joueur = i;
        (mes_args_tcp+i)->partie_en_cours = is_enCours;
        (mes_args_tcp+i)->mutex = mutex;
        (mes_args_tcp+i)->mutex_tchat = mutex_tchat;
        (mes_args_tcp+i)->joueurs = joueurs;
        (mes_args_tcp+i)->is_equipe = is_equipe;
        if(pthread_create(&les_threads[i], NULL, communication_tcp, (void *)(mes_args_tcp+i))){
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    struct game_args *partie_args = malloc(sizeof(struct game_args));
    assert(partie_args!=NULL);
    
    partie_args->socket_mdiff = sock_mdiff;
    partie_args->addr_mdiff = sock_adresse_mdiff;
    partie_args->inGame = is_enCours;
    partie_args->mutex = mutex;
    partie_args->joueurs = joueurs;
    partie_args->plateau = plateau;
    partie_args->is_partie_equipe=is_equipe;
    action* actions = malloc(sizeof(action)*8);
    assert(actions!=NULL);
    for(int i =0; i<8; i++){
        (*(actions+i)).code=-1;
    }
    partie_args->actions = actions;

    if(pthread_create(&les_threads[4], NULL, game_serv, (void *)partie_args)){
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    //thread reception données
    struct recv_args *req_args = malloc(sizeof(struct recv_args));
    assert(req_args!=NULL);
    
    req_args->sock_udp = sock_udp;
    req_args->partie_en_cours = is_enCours;
    req_args->mutex = mutex;
    req_args->actions = actions;
    req_args->joueurs = joueurs;
    if(pthread_create(&les_threads[5], NULL, action_request, (void *)req_args)){
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 6; ++i) {
        pthread_join(les_threads[i], NULL);
    }

    free(is_enCours);
    free(mutex);
    free(mutex_tchat);
    free(partie);
    free_board(plateau);
    free(plateau);
    free(joueurs);
    free(actions);
    free(partie_args);
    free(req_args);
    free(mes_args);

    return NULL;

}

/* gère les demandes d'action des clients */
void* action_request(void* args){

    struct recv_args* mes_args = (struct recv_args*)(args);
    action* actions = mes_args->actions;
    int sock_udp = mes_args->sock_udp;
    pthread_mutex_t* mutex = mes_args->mutex;
    bool* partie_en_cours = mes_args->partie_en_cours;
    joueur* joueurs = mes_args->joueurs;

    for(size_t i =0; i<4; ++i) {
        pthread_mutex_lock(mutex);
        joueurs[i].afk = clock();
        pthread_mutex_unlock(mutex);
    }

    while(1){

        // si le joueur est afk, on le tue
        for(size_t i = 0; i<4; ++i) {
            pthread_mutex_lock(mutex);
            clock_t last_act = joueurs[i].afk;
            pthread_mutex_unlock(mutex);
            clock_t now = clock();
            int seconds_afk = (now-last_act) / CLOCKS_PER_SEC;
            if(seconds_afk>=120) {
                pthread_mutex_lock(mutex);
                joueurs[i].status = -1;
                pthread_mutex_unlock(mutex);
            }
        }

        pthread_mutex_lock(mutex);
        if(!(*partie_en_cours)){
            pthread_mutex_unlock(mutex);
            break;
        }
        pthread_mutex_unlock(mutex);
        //recevoir les demandes des joueurs
        char buf[4];
        int rcv = recvfrom(sock_udp, buf, 4, MSG_DONTWAIT,NULL,NULL);
        if (rcv < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                // il n'y a rien a lire
                continue;
            }
            perror("erreur recv demandes d'action");
            continue;
        }
        int* mess = deformat_action(buf);
        int id_joueur = mess[1];
        pthread_mutex_lock(mutex);
        joueurs[id_joueur].afk = clock();
        pthread_mutex_unlock(mutex);
        action act;
        if(mess[4]==4){
            pthread_mutex_lock(mutex);
            if((*(actions+id_joueur*2+1)).code==-1 || (*(actions+id_joueur*2+1)).num<mess[3] || ((*(actions+id_joueur*2+1)).num>60000 && mess[3]<1000)){
                act.code=4;
                act.num=mess[3];
                *(actions+mess[1]*2+1)=act;
            }
            pthread_mutex_unlock(mutex);
        }
        else if(mess[4]==5){
            pthread_mutex_lock(mutex);
            if((*(actions+id_joueur*2)).code==-1 || (*(actions+id_joueur*2)).num<mess[3] || ((*(actions+id_joueur*2)).num>60000 && mess[3]<1000)){
                (*(actions+id_joueur*2)).code = 5;
            }
            pthread_mutex_unlock(mutex);
        }
        else{
            pthread_mutex_lock(mutex);
            if((*(actions+id_joueur*2)).code==-1 || (*(actions+id_joueur*2)).num<mess[3] || ((*(actions+id_joueur*2)).num>60000 && mess[3]<1000)){
                act.code=mess[4];
                act.num=mess[3];
                *(actions+mess[1]*2)=act;
            }
            pthread_mutex_unlock(mutex);
        }
        free(mess);
    }
    close(sock_udp);
    return NULL;
}

/* gère le chat pour 1 client */
void *communication_tcp(void *args) {

    struct tcp_thread_args *mes_args = (struct tcp_thread_args *)args;
    int* partie = mes_args->partie;
    int id = mes_args->id_joueur;
    int sock = mes_args->partie[id];
    bool* partie_en_cours = mes_args->partie_en_cours;
    pthread_mutex_t* mutex = mes_args->mutex;
    pthread_mutex_t* mutex_tchat = mes_args->mutex_tchat;
    struct joueur* joueurs = mes_args->joueurs;
    bool is_equipe = mes_args->is_equipe;
    
    while(1) {
        pthread_mutex_lock(mutex);
        if(!*partie_en_cours){
            pthread_mutex_unlock(mutex);
            break;
        }
        pthread_mutex_unlock(mutex);
        char* buf = malloc(3);
        assert(buf!=NULL);
        int recu = 0;
        while(recu<3) {
            pthread_mutex_lock(mutex_tchat);
            int rcv = recv(sock, buf+recu, 3-recu, MSG_DONTWAIT);
            pthread_mutex_unlock(mutex_tchat);
            if (rcv < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    // il n'y a rien a lire
                    break;
                }
                perror("erreur recv tchat entete");
                break;
            }
            else if(rcv==0){//au cas où pas de message
                break;
            }
            recu+=rcv;
        }
        if(recu==0){
            free(buf);
            continue;
        }
        
        int* msg_deformate = NULL;
        msg_deformate = deformat_entete_tchat(buf);
        assert(msg_deformate!=NULL);
        free(buf);
        int len = msg_deformate[3];
        char* buf2 = malloc(len);
        assert(buf2!=NULL);
        recu = 0;
        
        while(recu<len) {
            pthread_mutex_lock(mutex_tchat);
            int rcv = recv(sock, buf2+recu, len-recu, 0);
            pthread_mutex_unlock(mutex_tchat);
            if (rcv < 0) {
                perror("erreur recv tchat ");
                continue;
            }
            recu+=rcv;
        }
        if(msg_deformate[0]==7){
            int codereq = 13;
            char* message = format_tchat(codereq, msg_deformate[1], msg_deformate[2], len, buf2);

            free(buf2);
            for(size_t i = 0; i<4; ++i) {
                if (i!=id) {
                    int env = 0;
                    pthread_mutex_lock(mutex_tchat);
                    while(env<len+3) {
                        int envoye = send(partie[i], message+env, len+3-env, 0);
                        if (envoye < 0) {
                            perror("erreur send");
                            continue;
                        }
                        env+=envoye;
                    }
                    pthread_mutex_unlock(mutex_tchat);
                }
            }
            free(message);
        }
        else if(msg_deformate[0]==8){
            int codereq = 14;
            char* message = format_tchat(codereq, msg_deformate[1], msg_deformate[2], len, buf2);
            free(buf2);
            for(size_t i = msg_deformate[2]*2; i<msg_deformate[2]*2+2; ++i) {
                if (i!=id) {
                    int env = 0;
                    pthread_mutex_lock(mutex_tchat);
                    while(env<len+3) {
                        int envoye = send(partie[i], message+env, len+3-env, 0);
                        if (envoye < 0) {
                            perror("erreur send");
                            continue;
                        }
                        env+=envoye;
                    }
                    pthread_mutex_unlock(mutex_tchat);
                }
            }
            free(message);
        }
                       
        
        free(msg_deformate);
    }

    // on envoie le msg "partie terminée" formaté au client
    int env = 0;
    int eq_win = 0;
    int id_win = 0;
    if(is_equipe){
        pthread_mutex_lock(mutex);
        if ((*(joueurs+0)).status==-1 && (*(joueurs+1)).status==-1) {
            eq_win = 1;
        }
        if ((*(joueurs+2)).status==-1 && (*(joueurs+3)).status==-1) {
            eq_win = 0;
        }
        pthread_mutex_unlock(mutex);
    }
    else {
        pthread_mutex_lock(mutex);
        for (int i = 0; i < 4; i++) {
            if ((*(joueurs+i)).status!=-1) {
                id_win = i;
            }
        }
        pthread_mutex_unlock(mutex);
    }

    int codereq = is_equipe ? 16 : 15;

    char* buf = format_entete(codereq,id_win,eq_win);
    pthread_mutex_lock(mutex_tchat);

    while(env<2) {//MUTEX
        int envoye = send(sock, buf+env, 2-env, 0);
        if (envoye < 0) {
            perror("erreur send");
            continue;
        }
        env+=envoye;
    }
    fprintf(stderr, "fin de partie\n");
    
    free(buf);
    pthread_mutex_unlock(mutex_tchat);

    return NULL;
}

/* logique du jeu et multidiffusion de la grille */
void* game_serv(void* args){

    struct game_args* mes_args = (struct game_args*)(args);
    assert(mes_args!=NULL);
    struct board* plateau = mes_args->plateau;
    struct joueur* joueurs = mes_args->joueurs;
    bool is_partie_equipe = mes_args->is_partie_equipe;
    
    struct board* delay_board = malloc(sizeof(board));
    assert(delay_board!=NULL);
    delay_board->grid = malloc(plateau->h*plateau->w);
    assert(delay_board->grid!=NULL);
    memcpy(delay_board->grid, plateau->grid, plateau->h*plateau->w);
    delay_board->h=plateau->h;
    delay_board->w=plateau->w;
    //actions des joueurs
    //le dernier mouvement demandé par le joueur id est stocké à actions[id*2] et sa derniere demande de bombe à actions[id*2+1]
    struct action* actions = mes_args->actions;
    clock_t current_time;
    clock_t time_at_freq=clock();
    clock_t time_at_sec=clock()-CLOCKS_PER_SEC; // soustraction pour que la boucle envoie le plateau entier dès la première itération
    int num = 0;

    assert(joueurs!=NULL);

    while(1){
        int nb_morts=0;
        for(int i = 0; i<4; i++){//verifier si tous les joueurs sont KO
            if((*(joueurs+i)).status==-1) nb_morts++;
        }

        if(is_partie_equipe){
            if(nb_morts>=2) {
                if((( (*(joueurs+0)).status==-1 ) && ( (*(joueurs+1)).status==-1 ))
                   || (( (*(joueurs+2)).status==-1 ) && ( (*(joueurs+3)).status==-1 ))) {
                    pthread_mutex_lock(mes_args->mutex);
                    *(mes_args->inGame)=false;
                    pthread_mutex_unlock(mes_args->mutex);
                    break; 
                }
            }
        }
        else {
            if(nb_morts>=3){
                pthread_mutex_lock(mes_args->mutex);
                *(mes_args->inGame)=false;
                pthread_mutex_unlock(mes_args->mutex);
                break;
            }
        }
        current_time=clock();
        int freq_time_diff = ((double)(current_time - time_at_freq) / CLOCKS_PER_SEC) *1000;
        int sec_time_diff = ((double)(current_time - time_at_sec) / CLOCKS_PER_SEC) *1000;

        if(freq_time_diff>=FREQ){
            //enlever zone de danger
            update_dangerzone(delay_board);
            //update les bombes
            for(int i=0; i<4; i++){
                pthread_mutex_lock(mes_args->mutex);
                update_bomb_player((joueurs+i), delay_board, freq_time_diff);
                pthread_mutex_unlock(mes_args->mutex);
            }

            //est ce que des gens sont morts
            for(int i=0; i<4; i++){
                pthread_mutex_lock(mes_args->mutex);
                playerDead((joueurs+i), delay_board);
                pthread_mutex_unlock(mes_args->mutex);
            }
            
            //traiter les derniere demandes des joueurs
            for(int i=0; i<4; i++){
                for(int j = 0; j<2; j++){
                    pthread_mutex_lock(mes_args->mutex);
                    if((*(joueurs+i)).status!=-1){
                        int code = (*(actions+i*2+j)).code;
                        if(code!=-1){
                            perform_action(delay_board, (joueurs+i), *(actions+i*2+j));
                            (actions+i*2+j)->code_action_precedente=code;
                            (actions+i*2+j)->code=-1;
                        }
                    }
                    pthread_mutex_unlock(mes_args->mutex);
                }
            }
            
            //send partial board
            int* nb = malloc(sizeof(int));
            assert(nb!=NULL);
            *nb = -1;
            case_t* changes = changements(plateau,delay_board,nb);
            char* message = format_partialgrid(12,0,0,num,*nb,changes);
            num = (num + 1)%65536;
            int taille_partial_grid = 5+3*(*nb);
            int envoye = sendto(mes_args->socket_mdiff, message, taille_partial_grid, 0, (struct sockaddr *)&(mes_args->addr_mdiff), sizeof((mes_args->addr_mdiff)));
            if (envoye < 0) {
                free(changes);
                free(message);
                free(nb);
                perror("erreur send partial grid");
                break;
            }
            free(changes);
            free(message);
            free(nb);
            memcpy(plateau->grid,delay_board->grid, plateau->h*plateau->w);

            time_at_freq= clock();
        }

        if(sec_time_diff>=1000){
            //send complete board
            char* message = format_fullgrid(11,0,0,num,plateau);
            num = (num + 1)%65536;
            int taille = plateau->h*(plateau->w)+6;
            
            int envoye = sendto(mes_args->socket_mdiff, message, taille, 0, (struct sockaddr *)&(mes_args->addr_mdiff), sizeof((mes_args->addr_mdiff)));
            if (envoye < 0) {
                perror("erreur send fullgrid ");
                break;
            }

            free(message);
            time_at_sec = clock();
        }
    }
    
    //free
    free_board(delay_board);
    free(delay_board);

    return NULL;
}