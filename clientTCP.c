#include "clientTCP.h"

/*fonction qui initialise la socket TCP et 
renvoie une structure contenant les valeurs 
de l'entete le portUdp et multidiffusé et l'adresse */
struct adresse_mdiff socket_client(char* port, char* address, int codereq,int* sock) { 
    // Initialisation des structures addrinfo
    struct addrinfo hints, *p, *r;
    int ret;    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6; 
    hints.ai_socktype = SOCK_STREAM;
     
    // Appel à getaddrinfo
    if ((ret = getaddrinfo(address, port, &hints, &r)) != 0){
        fprintf(stderr, "Erreur getaddrinfo : %s\n", gai_strerror(ret));
        exit(1);
    }

    // Recherche d'une connexion valide
    for (p = r; p != NULL; p = p->ai_next) {
        *sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (*sock > 0 && connect(*sock, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        close(*sock);
    }

    if (p == NULL) {
        fprintf(stderr, "Aucune connexion valide trouvée.\n");
        freeaddrinfo(r);
        exit(1);
    }

    printf("Connecté\n");
    freeaddrinfo(r);

    // Envoi des données de l'entete
    char* buf = format_entete(codereq, 0, 0);
    int envoi = 0;

    while(envoi<2) {
      int env = send(*sock, buf+envoi, 2-envoi, 0);
      
      if(env == -1) {
        perror("Erreur send");
        close(*sock);
        free(buf);
        exit(1);
      }
      envoi+=env;
    }
    free(buf);
    
    // Réception du premier message 
    char buf2[22];
    size_t recu = 0;
    while (recu < 22) {
        int rcv = recv(*sock, buf2 + recu, 22 - recu, 0); // En mode bloquant
        if (rcv == 0) {
            printf("Le serveur a fermé la connexion.\n");
            break;
        } else if (rcv < 0) {
            perror("Erreur recv");
            close(*sock);
            exit(1);
        }
        recu += rcv;
    }

    printf("Réception complète : %s\n", buf2);
    struct adresse_mdiff res = deformat_adress_mdiff(buf2);

    return res;
}