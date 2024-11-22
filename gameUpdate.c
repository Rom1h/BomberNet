#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "type.h"
#include "board.h"
#include "gameUpdate.h"
#define DETONATE_TIME 5000

void explode_bomb(struct board* jeu,struct bombe* bombe);

int action_valide(action a, board * b,joueur * j){
    int val;
    int xd = 0;
    int yd = 0;
    switch (a.code) {
        case 3:    
            xd = -1; yd = 0;val=3; break;
        case 1:
            xd = 1; yd = 0; val=1;break;
        case 0:
            xd = 0; yd = -1; val=0; break;
        case 2:
            xd = 0; yd = 1; val=2;break;
        case 4 :
        
        for (int i = 0; i < 2; i++) {
            if (j->listeBombe[i].status == 0) { // Bombe disponible pour être posée
                j->listeBombe[i].status = 1; // Bombe en cours
                j->listeBombe[i].position.x = j->posJ.x;
                j->listeBombe[i].position.y = j->posJ.y;
                j->listeBombe[i].time_left = DETONATE_TIME; 
                set_grid(b, j->posJ.x, j->posJ.y, 3); // Affiche la bombe sur la grille

                break;
            }
        }   
            return -1;
        case -1:
            return -2;
        default: break;
    }
    
    int newX = j->posJ.x + xd;
    int newY = j->posJ.y + yd;

    //On verifie si le joueurs est dans les limites du terrain
    if (newX >= 0 && newX < b->w && newY >= 0 && newY < b->h) {
        int gridValue = get_grid(b, newX, newY);
        //on verifie si il y a un mur ou une bombe 
        if (gridValue != 1 && gridValue != 2 && gridValue != 3) {
            j->posJ.x = newX;
            j->posJ.y = newY;
            return val;
        }
        else return -1;
    }
    else return -1;
}

//Fonction qui gère les touches entrée par l'utilisateur 
int control(line* l) {
    int c;
    int prev_c = ERR;
    
    while ((c = getch()) != ERR) { 

        if (prev_c != ERR) {
            ungetch(c); 
            break;
        }
        prev_c = c;
    }
                
    int a = -1;
    switch (prev_c) {
        case ERR: break;
        case KEY_LEFT:
            a = 3; break;
        case KEY_RIGHT:
            a = 1; break;
        case KEY_UP:
            a = 0; break;
        case KEY_DOWN:
            a = 2; break;
        case '*':
            a = 4; break;
        case '&':
            a = 5; break;
        case 22:
            a = -1; break;
        case KEY_BACKSPACE:
            if (l->cursor > 0){
            l->data[(l->cursor)-1] = '\0';
            update_input_line(l);}
             l->cursor--;
            
            break;
        case 127 :
            if (l->cursor > 0){l->data[(l->cursor)-1] = '\0';
            update_input_line(l);
            --l->cursor;
            }
            break;
        case 10 :
           return 6;
        default:
            if (prev_c >= ' ' && prev_c <= '~' && l->cursor < TEXT_SIZE){
                l->data[(l->cursor)++] = prev_c;
                update_input_line(l);
            }
            break;
    }
    return a;
}

//maj du jeu dans le serveur
bool perform_action(board* b, joueur* player, action a) {
    
    if(get_grid(b,player->posJ.x,player->posJ.y)==4){
        player->status=-1;
        return true;
    }

    int xd = 0;
    int yd = 0;
    switch (a.code) {
        case 3:      
            xd = -1; break;
        case 1:
            xd = 1; break;
        case 0:
            yd = -1; break;
        case 2:
            yd = 1; break;
        case 4 :
        
        for (int i = 0; i < 2; i++) {
            if (player->listeBombe[i].status == 0) { // Bombe disponible pour être posée
                player->listeBombe[i].status = 1; // Bombe en cours
                player->listeBombe[i].position.x = player->posJ.x;
                player->listeBombe[i].position.y = player->posJ.y;
                player->listeBombe[i].time_left = DETONATE_TIME; 
                set_grid(b, player->posJ.x, player->posJ.y, 3); // Affiche la bombe sur la grille

                break;
            }
        }   
            return true;
        case 5 : 
            int prev_code = a.code_action_precedente;
            if(prev_code==0) yd = 1;
            if(prev_code==1) xd = -1;
            if(prev_code==2) yd = -1;
            if(prev_code==3) xd = 1;
            break;
        case -1:
            return false;
        default: break;
    }
    
    int newX = player->posJ.x + xd;
    int newY = player->posJ.y + yd;

    //On verifie si le joueurs est dans les limites du terrain
    if (newX >= 0 && newX < b->w && newY >= 0 && newY < b->h) {
        int gridValue = get_grid(b, newX, newY);
        //on verifie si il y a un mur ou une bombe 
        if (gridValue != 1 && gridValue != 2 && gridValue != 3) {
            if(get_grid(b, player->posJ.x, player->posJ.y)!=4 && get_grid(b, player->posJ.x, player->posJ.y)!=3){
                set_grid(b, player->posJ.x, player->posJ.y, 0); // Marquer l'ancienne position
            }
            player->posJ.x = newX;
            player->posJ.y = newY;
            if(get_grid(b, player->posJ.x, player->posJ.y)!=4){
                set_grid(b, player->posJ.x, player->posJ.y, 5+player->id); // Marquer la nouvelle position
            }

            return true;
            
        }

    }
    return false;
}

//mets à jour le temps restant pour chaque bombe
void update_bomb_player(struct joueur* player,struct board* jeu, int time) {
    for (int i = 0; i < 2; i++) {
        if(player->listeBombe[i].status==1){
            if (player->listeBombe[i].time_left > 0) {
                player->listeBombe[i].time_left-=time;
                if (player->listeBombe[i].time_left <= 0) {
                    explode_bomb(jeu, &player->listeBombe [i]);
                }
            }
        }
    }
}

//enleve les zone de danger
void update_dangerzone(struct board* jeu){
    for(int x = 0; x<jeu->w; x++){
        for(int y = 0; y<jeu->h; y++){
            if(get_grid(jeu,x,y)==4){
                set_grid(jeu,x,y,0);
            }
        }
    }
}


void explode_bomb(struct board* jeu, struct bombe* bombe) {
    int x = bombe->position.x;
    int y = bombe->position.y;
    bombe->status = 0;  

    // Haut
    for (int i = 0; i < 3; i++) {
        if (y - i < 0 || get_grid(jeu, x, y - i) == 1) break;
        set_grid(jeu, x, y - i, 4);
        if (y - i < 0 || get_grid(jeu, x, y - i) == 1) break;
        set_grid(jeu, x, y - i, 4);
    }

    // Bas
    for (int i = 0; i < 3; i++) {
        if (y + i >= jeu->h || get_grid(jeu, x, y + i) == 1) break;
        set_grid(jeu, x, y + i, 4);   
    }

    // Gauche
    for (int i = 0; i < 3; i++) {
        if (x - i < 0 || get_grid(jeu, x - i, y) == 1) break;
        set_grid(jeu, x - i, y, 4);
    }

    // Droite
    for (int i = 0; i < 3; i++) {
        if (x + i >= jeu->w || get_grid(jeu, x + i, y) == 1) break;
        set_grid(jeu, x + i, y, 4);
        if (x + i >= jeu->w || get_grid(jeu, x + i, y) == 1) break;
        set_grid(jeu, x + i, y, 4);
       
    }

    // Diagonales
    if (x + 1 < jeu->w && y + 1 < jeu->h && get_grid(jeu, x + 1, y + 1) != 1) {
        set_grid(jeu, x + 1, y + 1, 4);
       
    }
    if (x - 1 >= 0 && y - 1 >= 0 && get_grid(jeu, x - 1, y - 1) != 1) {
        set_grid(jeu, x - 1, y - 1, 4);
      
    }
    if (x + 1 < jeu->w && y - 1 >= 0 && get_grid(jeu, x + 1, y - 1) != 1) {
        set_grid(jeu, x + 1, y - 1, 4);
    }
    if (x - 1 >= 0 && y + 1 < jeu->h && get_grid(jeu, x - 1, y + 1) != 1) {
        set_grid(jeu, x - 1, y + 1, 4);
      
    }
    
}

//tue les joueurs dans les zones d'explosion
void playerDead(joueur* player,board * jeu){
    if(get_grid(jeu,player->posJ.x,player->posJ.y)==4){
        player->status=-1;
    }
}