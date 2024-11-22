#ifndef BOARD_H  
#define BOARD_H

#include "type.h"  


struct joueur* setup_board(board* b);
void free_board(board* b);
int get_grid(board* b, int x, int y);
void set_grid(board* b, int x, int y, int v);
struct joueur* new_joueur(int id, int w, int h);

#endif
