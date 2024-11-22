#ifndef GAMEVIEW_H  
#define GAMEVIEW_H
#include "type.h"

void update_input_line(line *send);
void delete_input_line(line *send);
void afficher_joueurGagnant(int id);
void afficher_EquipeGagnante(int id);
void refresh_game_affichage(board* b, line* send,Chat* chat,int equipe);
void init_View();

#endif