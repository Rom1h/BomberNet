#ifndef GAME_H
#define GAME_H

#include "type.h"  
#include "board.h" 
#include "gameView.h"
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void explode_bomb(struct board* jeu, struct bombe* bombe);
int control(line* l);
bool perform_action(board* b, joueur* player, action a);
void update_bomb_player(struct joueur* player, struct board* jeu,int time);
void update_dangerzone(struct board* jeu);
void playerDead(joueur* player,board * jeu);

#endif