#include "threads_serveur.h"

// génère une socket udp
int new_udp(int port){ 
    int sock_fd = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("création socket serveur udp échouée");
        exit(1);
    }

    struct sockaddr_in6 servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, ADDR, &servaddr.sin6_addr);
    servaddr.sin6_port = htons(port);

    if((bind(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)))<0) {
        perror("echec bind");
        exit(1);
    }

    return sock_fd;
}

// genère une socket de multidiffusion
int new_mdiff(int port, char* address, struct sockaddr_in6* servaddr) {
    int sock_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("création socket serveur udp échouée");
        exit(1);
    }
    
    memset(servaddr, 0, sizeof(*servaddr));
    servaddr->sin6_family = AF_INET6;
    servaddr->sin6_port = htons(port);
    inet_pton(AF_INET6, address, &servaddr->sin6_addr);
    int ifindex = 0;
    if(setsockopt(sock_fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)))
        perror("erreur initialisation de l’interface locale");

    return sock_fd;
}

// crée une adresse de multidiffusion
char* new_adress_mdiff(int i){
    char suf_add_mdiff[4];
    snprintf(suf_add_mdiff, sizeof(suf_add_mdiff), "%x", i);

    char* add_mdiff = malloc(INET6_ADDRSTRLEN);
    assert(add_mdiff!=NULL);
    snprintf(add_mdiff, INET6_ADDRSTRLEN, "%s%s", PREF_MULTICAST_ADDR, suf_add_mdiff);
    return add_mdiff;
}

int main(int argc, char** argv) {
    
    if(argc<2){
        fprintf(stderr,"Format incorrect\n");
        exit(1);
    }

    // Création socket serveur
    int socket_serveur = socket(PF_INET6, SOCK_STREAM, 0);
    if (socket_serveur < 0) {
        perror("création socket_serveur échouée");
        exit(1);
    }

    // serveur polymorphe 
    int no = 0;
    int r = setsockopt(socket_serveur, IPPROTO_IPV6, IPV6_V6ONLY, &no, sizeof(no));
    if (r < 0) {
        perror("échec de setsockopt");
        exit(1);
    }

    // Création adresse du destinataire
    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(atoi(argv[1]));
    address_sock.sin6_addr = in6addr_any;

    // on lie la socket au port
    r = bind(socket_serveur, (struct sockaddr *) &address_sock, sizeof(address_sock));
    if (r < 0) {
        perror("erreur bind");
        exit(2);
    }

    // le serveur est prêt à écouter les connexions 
    r = listen(socket_serveur, 0);
    if (r < 0) {
        perror("erreur listen");
        exit(1);
    }

    // Création socket client
    int* partie_solo = malloc(4*sizeof(int));
    assert(partie_solo!=NULL);
    int nbj_solo = 0;

    int* partie_equipe = malloc(4*sizeof(int));
    assert(partie_equipe!=NULL);
    int nbj_ekip = 0;

    // Serveurs UDP
    int port = 49152;
    int sock_udp_solo = new_udp(port);
    int port_UDP_solo = port;
    port+=1;
    int sock_udp_equipe = new_udp(port);
    int port_UDP_equipe = port;
    port+=1;

    // multidiff
    int cpt_mdiff = 0;
    char* adr_mdiff_solo = new_adress_mdiff(cpt_mdiff);
    int port_mdiff_solo = port;
    struct sockaddr_in6 sock_adresse_mdiff_solo; 
    int sock_mdiff_solo = new_mdiff(port_mdiff_solo, adr_mdiff_solo, &sock_adresse_mdiff_solo);
    cpt_mdiff+=1;
    port+=1;

    char* adr_mdiff_equipe = new_adress_mdiff(cpt_mdiff);
    int port_mdiff_equipe = port;
    struct sockaddr_in6 sock_adresse_mdiff_equipe; 
    int sock_mdiff_equipe = new_mdiff(port_mdiff_equipe, adr_mdiff_equipe, &sock_adresse_mdiff_equipe);
    cpt_mdiff+=1;
    
    while(1) {
        
        int* socket_client = malloc(sizeof(int));
        assert(socket_client!=NULL);
        
        *(socket_client) = accept(socket_serveur, NULL, NULL);

        if (*(socket_client) == -1) {
            perror("création socket_client échouée");
            free(socket_client);
            continue;
        }
        
        char* buf = malloc(2); // 16 bits 2 octets
        assert(buf!=NULL);
        memset(buf, 0, 2);
        int rec = 0;
        while(rec<2){
            int rcv = recv(*socket_client, buf+rec, 2-rec, 0);
            if (rcv < 0) {
                perror("erreur recv integration partie");
                continue;
            }
            rec+=rcv;
        }
        
        // création des parties solo et equipes
        int* entete = deformat_entete(buf);
        int codereq = entete[0]; 
        free(buf);
        free(entete);
        int id_joueur;
        int id_equipe = 0;

        // partie solo 
        if(codereq==1){ 
            // création d'une nouvelle partie si la dernière est pleine
            if(nbj_solo == 4 ) {
                port = (port!=65535) ? port+1 : 49152;
                sock_udp_solo = new_udp(port);
                port_UDP_solo = port;

                partie_solo = malloc(4*sizeof(int));
                assert(partie_solo!=NULL);
                nbj_solo = 0;

                port = (port!=65535) ? port+1 : 49152;
                sock_mdiff_solo = new_mdiff(port, adr_mdiff_solo, &sock_adresse_mdiff_solo);
                port_mdiff_solo = port;
                cpt_mdiff+=1;
                
            }
            // ajout du joueur à la partie
            *(partie_solo+nbj_solo) = *(socket_client);
            id_joueur = nbj_solo;
            nbj_solo+=1;
                
            // Envoi des infromations au client 
            char* a_envoyer =  format_adress_mdiff(9, id_joueur, 0, port_UDP_solo, port_mdiff_solo, adr_mdiff_solo);
            int t = 22;
            int sent = 0;
            while (sent<t) {
                
                int envoi = send(*socket_client, a_envoyer+sent, t-sent, 0);
                if (envoi < 0) {
                    perror("Erreur send info au client");
                    continue;
                }
                sent+=envoi;
            }
             
        }
        else if(codereq==2){
            // création d'une nouvelle partie si la dernière est pleine
            if(nbj_ekip == 4) {
                port = (port!=65535) ? port+1 : 49152;
                sock_udp_equipe = new_udp(port);
                port_UDP_equipe = port;

                partie_equipe = malloc(4*sizeof(int));
                assert(partie_equipe!=NULL);
                nbj_ekip = 0;

                port = (port!=65535) ? port+1 : 49152;
                adr_mdiff_equipe = new_adress_mdiff(cpt_mdiff);
                sock_mdiff_equipe = new_mdiff(port, adr_mdiff_equipe, &sock_adresse_mdiff_equipe);
                port_mdiff_equipe = port;
                cpt_mdiff+=1;
            }
            // ajout du joueur à la partie avec son num d'équipe
            *(partie_equipe+nbj_ekip) = *(socket_client);
            id_joueur = nbj_ekip;
            nbj_ekip+=1;
            id_equipe = (nbj_ekip<3) ? 0 : 1;

             // Envoi des infromations au client 
            char* a_envoyer =  format_adress_mdiff(10, id_joueur, id_equipe, port_UDP_equipe, port_mdiff_equipe, adr_mdiff_equipe);
            int t = 22;
            int sent = 0;
            while (sent<t) {
                int envoi = send(*socket_client, a_envoyer+sent, t-sent, 0);
                if (envoi < 0) {
                    perror("Erreur send info au client");
                    continue;
                }
                sent+=envoi;
            }
        }
        else {
            // erreur impossible
            continue;
        }

        // Réception confirmation du joueur
        char* buf_confirm = malloc(2);
        assert(buf_confirm!=NULL);
        int rec2 = 0;

        while(rec2<2){
            
            int rcv = recv(*socket_client, buf_confirm+rec2, 2-rec2, 0);
            if (rcv < 0) {
                perror("erreur recv confirmation");
                continue;
            }
            rec2+=rcv;
        }
                
        int* entete2 = deformat_entete(buf_confirm);
        codereq = entete2[0]; 
        free(buf_confirm);
        free(entete2);

        if(codereq==3) {
            
            // si la partie solo est pleine on la lance
            if(nbj_solo==4) {
                
                //thread start game
                pthread_t thread;
                struct start_game_args *mes_args = malloc(sizeof(struct start_game_args));
                assert(mes_args!=NULL);
                mes_args->partie = partie_solo;
                mes_args->sock_udp = sock_udp_solo;
                mes_args->sock_mdiff = sock_mdiff_solo;
                mes_args->sock_adresse_mdiff=sock_adresse_mdiff_solo;
                mes_args->is_equipe = false;
                if(pthread_create(&thread, NULL, start_game, (void *)mes_args)){
                    perror("pthread_create");
                    continue;
                }
            }
        }

        // si la partie équipe est pleine on la lance
        else if(codereq==4) {
            if(nbj_ekip==4) {

                //thread start game
                pthread_t thread;
                struct start_game_args *mes_args = malloc(sizeof(struct start_game_args));
                assert(mes_args!=NULL);
                
                mes_args->partie = partie_equipe;
                mes_args->sock_udp = sock_udp_equipe;
                mes_args->sock_mdiff = sock_mdiff_equipe;
                mes_args->sock_adresse_mdiff=sock_adresse_mdiff_equipe;
                mes_args->is_equipe = true;
                if(pthread_create(&thread, NULL, start_game, (void *)mes_args)){
                    perror("pthread_create");
                    continue;
                }
            }
        }
        
    }
    close(socket_serveur);
    return 0;
}