#include "formatage.h"

char* format_entete(uint16_t code_req, uint16_t id, uint16_t eq){
    uint16_t entete = 0;
    /*entete|=code_req; 
    entete|=(id<<13); 
    entete|=(eq<<15); */
    entete|=(code_req<<3);
    entete|=(id<<1); 
    entete|=(eq); 

    entete = htons(entete);
    char* message = malloc(2);
    assert(message!=NULL);
    memset(message, 0, 2);
    memcpy(message, &entete, sizeof(uint16_t));
    return message;
}

//renvoie un tableau contenant codereq id et eq
int* deformat_entete(char* message){
    assert(message!=NULL);
    uint16_t entete;
    memcpy(&entete, message, sizeof(uint16_t));
    entete = ntohs(entete);
    int* tab = malloc(3*sizeof(int));
    assert(tab!=NULL);
    /*
    //on recupere les 13 premiers bits du message
    uint16_t code_req = (entete<<3);
    tab[0]=(code_req>>3);
    //on recupere les bits 13 à 14
    uint16_t id = (entete>>13);
    tab[1] = id & 0x3;
    //on recup le dernier bit
    uint16_t eq = (entete>>15);
    tab[2]=eq;
    */
    //codereq
    tab[0]=(entete>>3);
    //on recupere les bits 13 à 14
    uint16_t id = (entete>>1);
    tab[1] = id & 0x3;
    //on recup le dernier bit

    uint16_t eq = (entete<<15);
    tab[2]=eq>>15;

    return tab;
}

void affiche_hexa(char* message, int t){
    for(int i=0;i<t;i++){
        fprintf(stderr,"mess : %02X\n",message[i]);
    }
}

char* format_action(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t num, uint16_t action){
    char* mess = malloc(4);
    assert(mess!=NULL);
    memset(mess, 0, 4);

    char* entete = format_entete(code_req,id,eq);
    memcpy(mess, entete, sizeof(uint16_t));
    free(entete);

    uint16_t suite = 0;
    suite |= (num<<3);
    suite |= (action);
    suite = htons(suite);

    memcpy(mess+2, &suite, sizeof(uint16_t));
    return mess;
}

//renvoie un tableau contenant codereq id eq num et le code de l'action
int* deformat_action(char* message){
    assert(message!=NULL);
    int* tab = malloc(5*sizeof(int));
    assert(tab!=NULL);
    int* entete = deformat_entete(message);
    memcpy(tab, entete, 3*sizeof(int));
    free(entete);

    uint16_t action;
    memcpy(&action, message+2,sizeof(uint16_t));
    action = ntohs(action);

    uint16_t num = (action>>3);
    tab[3] = (num);
    uint16_t act = (action<<13);
    tab[4] = (act>>13);

    return tab;
}

char* format_tchat(uint16_t code_req, uint16_t id, uint16_t eq, uint8_t len, char* data){
    //data doit finir par \0
    char* mess = malloc(3+len);
    assert(mess!=NULL);
    memset(mess, 0, 3+len);

    char* entete = format_entete(code_req,id,eq);
    memcpy(mess, entete, sizeof(uint16_t));
    free(entete);

    memcpy(mess+2, &len,sizeof(uint8_t));

    memcpy(mess+3, data, len);

    return mess;
}

//renvoie un tableau contenant codereq id eq et len
int* deformat_entete_tchat(char* message){
    int* tab = malloc(4*sizeof(int));
    memset(tab,0,4);
    assert(tab!=NULL);
    assert(message!=NULL);
    int* entete = deformat_entete(message);
    memcpy(tab, entete, 3*sizeof(int));
    free(entete);

    memcpy(tab+3, &message[2], sizeof(uint8_t));
    
    return tab;
}

char* format_adress_mdiff(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t portUDP, uint16_t portmdiff, char* adresse){
    char* mess = malloc(22);
    assert(mess!=NULL);
    memset(mess, 0, 22);

    char* entete = format_entete(code_req,id,eq);
    memcpy(mess, entete, sizeof(uint16_t));
    free(entete);
    
    portUDP = htons(portUDP);
    memcpy(mess+2, &portUDP,sizeof(uint16_t));

    portmdiff = htons(portmdiff);
    memcpy(mess+4, &portmdiff, sizeof(uint16_t));


    memcpy(mess+6, adresse, strlen(adresse));

    return mess;
}

//renvoie une structure contenant l'adresse mdiff, le port mdiff et le port udp
struct adresse_mdiff deformat_adress_mdiff(char* message){
    assert(message!=NULL);
    struct adresse_mdiff addr;
    addr.portUDP=0;
    addr.portmdiff=0;

    int* entete=deformat_entete(message); 
    memcpy(addr.entete, entete, 3*sizeof(int));
    free(entete);

    uint16_t portUDP;
    memcpy(&portUDP, message+2,sizeof(uint16_t));
    portUDP = ntohs(portUDP);
    memcpy(&addr.portUDP, &portUDP, sizeof(uint16_t));

    uint16_t portmdiff;
    memcpy(&portmdiff, message+4,sizeof(uint16_t));
    portmdiff = ntohs(portmdiff);
    memcpy(&addr.portmdiff, &portmdiff, sizeof(uint16_t));

    memcpy(addr.adresse, message+6,16);

    return addr;
}

char* format_fullgrid(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t num, board* plateau){
    char* mess = malloc(6 + (plateau->h * plateau->w));
    assert(mess!=NULL);
    memset(mess, 0, 6 + (plateau->h * plateau->w) );

    char* entete = format_entete(code_req,id,eq);
    memcpy(mess, entete, sizeof(uint16_t));
    free(entete);

    num = htons(num);
    memcpy(mess+2, &num, sizeof(uint16_t));

    uint8_t hauteur = plateau->h;
    memcpy(mess+4, &hauteur, sizeof(uint8_t));
       
    uint8_t largeur = plateau->w;
    memcpy(mess+5, &largeur, sizeof(uint8_t));

    int i=0;
    for(int y = 0; y<hauteur; y++){
        for(int x = 0; x<largeur; x++){
            uint8_t la_case = get_grid(plateau,x,y);
            memcpy(mess+6+i, &la_case, sizeof(uint8_t));
            i++;
        }
    }

    return mess;
}

char* format_partialgrid(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t num, uint8_t nb, case_t* changements){
    char* mess = malloc(5+3*nb);
    assert(mess!=NULL);
    memset(mess, 0, 5+3*nb);
    char* entete = format_entete(code_req,id,eq);
    memcpy(mess, entete, sizeof(uint16_t));
    free(entete);
    num = htons(num);
    memcpy(mess+2, &num,sizeof(uint16_t));
    memcpy(mess+4, &nb, sizeof(uint8_t));
    int i=0;
    for(int j = 0; j<nb; j++){
        case_t c = *(changements+j);
        uint8_t y = c.position.y; 
        memcpy(mess+5+i, &y, sizeof(uint8_t));
        i++;

        uint8_t x = c.position.x; 
        memcpy(mess+5+i, &x, sizeof(uint8_t));
        i++;

        uint8_t v = c.valeur; 
        memcpy(mess+5+i, &v, sizeof(uint8_t));
        i++;
    }
    return mess;
}

//renvoie un tableau contenant codereq, id, eq, num et nb le nombre de case qui ont changé
int* deformat_partialgrid_1(char* mess){
    assert(mess!=NULL);
    int * tab = malloc(2*sizeof(int));
    assert(tab!=NULL);
    uint16_t num;
    memcpy(&num,mess+2,2);
    memcpy(&num,mess+2,2);
    uint8_t nb;
    memcpy(&nb,mess+4,1);
    memcpy(&nb,mess+4,1);
    tab[0]=ntohs(num);
    tab[1]=nb;
    return tab;
}

//mess contient les cases et la fonctions mets à jour le plateau en fonction d'elle
void deformat_partialgrid_2(char* mess,board * board,int nb){
    assert(mess!=NULL);

    int i=0;
    for(int j = 0; j<nb; j++){

        uint8_t y; 
        memcpy( &y, mess+5+i, sizeof(uint8_t));
        memcpy( &y, mess+5+i, sizeof(uint8_t));
        i++;

        uint8_t x; 
        memcpy( &x, mess+5+i, sizeof(uint8_t));
        memcpy( &x, mess+5+i, sizeof(uint8_t));
        i++;

        uint8_t v; 
        memcpy( &v, mess+5+i, sizeof(uint8_t));
        memcpy( &v, mess+5+i, sizeof(uint8_t));
        i++;

        set_grid(board,x,y,v);

    }


}

//donne la largeur et la hauteur
int* deformat_fullgrid_1(char* mess){
    assert(mess!=NULL);
    uint16_t num;
    memcpy(&num, mess+2,sizeof(uint16_t));
    num = ntohs(num);

    uint8_t hauteur;
    memcpy(&hauteur, mess+4,sizeof(uint8_t));

    uint8_t largeur;
    memcpy(&largeur, mess+1,sizeof(uint8_t));

    int * size=malloc(3*sizeof(int));
    assert(size!=NULL);
    size[0]=num;
    size[1]=hauteur;
    size[2]=largeur;
    return size;
}
//maj de la grille en fonction des données de mess
void deformat_fullgrid_2(char* mess, board* plateau){
    assert(mess!=NULL);
    int i=0;
    for(int y = 0; y<plateau->h; y++){
        for(int x = 0; x<plateau->w; x++){
            uint8_t la_case;
            memcpy(&la_case,mess+6+i, sizeof(uint8_t));
            i++;
            set_grid(plateau,x,y,la_case);
        }
    }
}

/*
int main(){
    board* b;
    setup_board(b);
    char* lala=format_fullgrid(0,0,0,0, b);
    for(int i=0;i<3;i++){
        printf("%02X\n",(unsigned char)lala[i]);
    }
    deformat_fullgrid_2(lala+6,b);
    //printf("%d %d %d %d %d",entete[0], entete[1], entete[2],entete[3],entete[4]);
}
*/