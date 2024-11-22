#include "type.h"
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "board.h"
#define WIDTH 16
#define HEIGHT 16

/* initialise la grille du jeu */
struct joueur* setup_board(board* board) {
    assert(board!=NULL);
    // 2 lignes pour la bordure 1 pour le chat
    board->h = HEIGHT; 
    // 2 colonnes pour bordure
    board->w =  WIDTH; 
    board->grid = calloc((board->w)*(board->h),sizeof(char));
    assert(board->grid!=NULL);
    srand(time(NULL)); 
    // Initialise le generateur de nombres aléatoires
    for(int i =0;i<board->w;i++){
        for(int j =0;j<board->h;j++){
            int random_number = rand() % 10;
            if(random_number==1)
                set_grid(board,i,j,1);
            else{
                set_grid(board,i,j,2);
            }
        }

    }
    int w = board ->w;
    int h = board->h;

    //top left
    set_grid(board,0,0,5);
    set_grid(board,1,0,0);
    set_grid(board,0,1,0);
    set_grid(board,2,0,0);
    set_grid(board,0,2,0);
    set_grid(board,3,0,0);
    set_grid(board,0,3,0);

    //top right
    set_grid(board,w-1,0,5+2);
    set_grid(board,w-2,0,0);
    set_grid(board,w-1,1,0);
    set_grid(board,w-3,0,0);
    set_grid(board,w-1,2,0);
    set_grid(board,w-4,0,0);
    set_grid(board,w-1,3,0);
    
    //bottom left
    set_grid(board,0,h-1,5+1);
    set_grid(board,1,h-1,0);
    set_grid(board,0,h-2,0);
    set_grid(board,2,h-1,0);
    set_grid(board,0,h-3,0);
    set_grid(board,3,h-1,0);
    set_grid(board,0,h-4,0);

    //bottom right
    set_grid(board,w-1,h-1,5+3);
    set_grid(board,w-2,h-1,0);
    set_grid(board,w-1,h-2,0);
    set_grid(board,w-3,h-1,0);
    set_grid(board,w-1,h-3,0);
    set_grid(board,w-4,h-1,0);
    set_grid(board,w-1,h-4,0);

    struct joueur* joueurs = malloc(4*sizeof(joueur));
    assert(joueurs!=NULL);
    for(int i = 0; i<4; i++){
        joueur* j = new_joueur(i,w,h);
        *(joueurs+i)=*j;
        free(j);
    }

    return joueurs;
}

/* renvoie un joueur initialisé */
struct joueur* new_joueur(int id, int w, int h){
    struct joueur* j = malloc(sizeof(joueur));
    j->id=id;

    switch(id){
        case 0: j->eq=0; j->posJ.x=0; j->posJ.y=0; break;
        case 1: j->eq=0; j->posJ.x=0; j->posJ.y=h-1; break;
        case 2: j->eq=1; j->posJ.x=w-1; j->posJ.y=0; break;
        case 3: j->eq=1; j->posJ.x=w-1; j->posJ.y=h-1; break;
    }

    struct bombe b1;
    b1.status=0;
    struct bombe b2;
    b2.status=0;
    j->listeBombe[0]=b1;
    j->listeBombe[1]=b2;

    return j;
}

void free_board(board* board) {
    free(board->grid);
}

int get_grid(board* b, int x, int y) {
    return b->grid[y*b->w + x];
}

void set_grid(board* b, int x, int y, int v) {
    b->grid[y*b->w + x] = v;
}