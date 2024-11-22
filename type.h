#ifndef TYPES_H  
#define TYPES_H
#include <time.h>
#define MAX_MESSAGE_LENGTH 256
#define TEXT_SIZE 255
#define MAX_MESSAGES 100


typedef enum ACTION { NONE, UP, DOWN, LEFT, RIGHT, DROP_BOMB,CANCEL, QUIT } ACTION;

typedef struct {
    char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH];
    int message_count;
} Chat;

typedef struct action {
    int code_action_precedente;
    int num;
    int code;
    /*0 pour un déplacement d'une case_t vers le nord,
    1 pour un déplacement d’une case_t vers l'est,
    2 pour un déplacement d’une case_t vers le sud,
    3 pour un déplacement d’une case_t vers l'ouest,
    4 pour le dépôt d'une bombe,*/
}action;

typedef struct pos {
    int x;
    int y;
} pos;

typedef struct case_t {
    pos position;
    int valeur;
} case_t;

typedef struct bombe {
    pos position;
    int status;  // 0 dispo, -1 explose, 1 en cours
    int time_left;
} bombe;

typedef struct joueur {
    pos posJ;
    bombe listeBombe[2];
    int status;  // 1 en vie, -1 mort
    int id;
    int eq;
    clock_t afk; // temps en secondes depuis la dernière fois que le joueur a été actif
} joueur;

typedef struct board {
    char* grid;
    int w;
    int h;
} board;

typedef struct game {
    board board;
    joueur joueurs[4]; 
} game;

typedef struct line {
    char data[TEXT_SIZE];
    int cursor;
} line;

#endif 
