// Build with -lncurses option

#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "type.h"
#include "gameUpdate.h"
//Affichage du message que le client est en train d'écrire
void update_input_line(line *send) {
    mvprintw(LINES - 2, 1, "Message: %s", send->data);
    move(LINES - 2, 10 + send->cursor);
    refresh();
}
//Supprime le message entrée par l'utilisateur (quand il presse entrée)
void delete_input_line(line *send) {

    mvprintw(LINES - 2, 1, "%*s", COLS - 2, " ");

    // Afficher le nouveau message
    mvprintw(LINES - 2, 1, "Message: %s", send->data);

    // Déplacer le curseur à la position appropriée
    move(LINES - 2, 10 + send->cursor);

    // Rafraîchir l'écran pour appliquer les changements
    refresh();
}
//Affichage fin de partie en mode solo
void afficher_joueurGagnant(int id) {
    clear();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    char buf[50];
    memset(buf,0,50);
    sprintf(buf,"Le Joueur %d à gagné la partie !!",id);
    int x = (cols - strlen(buf)) / 2;
    int y = rows / 2;

    mvprintw(y, x, "%s", buf);

    refresh();
}
//Affichage fin de partie en mode équipe

void afficher_EquipeGagnante(int id) {
    clear();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    char buf[50];
    memset(buf,0,50);
    sprintf(buf,"L'équipe %d à gagné la partie !!",id);
    int x = (cols - strlen(buf)) / 2;
    int y = rows / 2;

    mvprintw(y, x, "%s", buf);

    refresh();
}

// Rafraîchit l'affichage avec les mise à jour du board et du chat
void refresh_game_affichage(board* b, line* send,Chat* chat,int equipe) {
    // maj grid
    int x,y;
    for (y = 0; y < b->h; y++) {
        for (x = 0; x < b->w; x++) {
            char c;
             int color_pair = 0;
            switch (get_grid(b,x,y)) {
               
                case 0:
                    c = ' '; // Pas de mur
                    break;
                case 1:
                    c = 'X'; // Mur Indestructible
                    break;
                case 2:
                    c = '#'; // Mur destructible
                    break;
                case 3:
                    c = '*'; // Bombe
                    break;
                case 4:
                    c = '?'; // Zone de danger
                    break;
                case 5:
                    c = 'O'; // Joueur 1
                    if(equipe==1) color_pair = 1;
                    else color_pair = 1;
                    break;
                case 6:
                    c = 'O'; // Joueur 2
                     if(equipe==1) color_pair = 2;
                    else color_pair = 1;
                    break;
                case 7:
                    c = 'O'; // Joueur 3
                     if(equipe==1) color_pair = 3;
                    else color_pair = 2;
                    break;
                case 8:
                    c = 'O'; // Joueur 4
                     if(equipe==1) color_pair = 4;
                    else color_pair = 2;
                    break;
                default:
                    c = '-';
                    break;
            }
            if (color_pair > 0) {
                attron(COLOR_PAIR(color_pair));
            }
            mvaddch(y + 1, x + 1, c);
            if (color_pair > 0) {
                attroff(COLOR_PAIR(color_pair));
            }
        
        }
    }
    for (x = 0; x < b->w+2; x++) {
        mvaddch(0, x, '-');
        mvaddch(b->h+1, x, '-');
    }
    for (y = 0; y < b->h+2; y++) {
        mvaddch(y, 0, '|');
        mvaddch(y, b->w+1, '|');
    }

    // maj chat
    int chat_start = b->h + 3;
    int chat_lines = LINES - chat_start - 3;
    int start_message = (chat->message_count > chat_lines) ? chat->message_count - chat_lines : 0;
    
    for (y = chat_start; y < chat_start + chat_lines; y++) {
        int msg_index = start_message + (y - chat_start);
        if (msg_index < chat->message_count) {
            mvprintw(y, 1, "%s", chat->messages[msg_index]);
        } else {
            break;
        }
    }

    update_input_line(send);
    refresh(); 
    attroff(A_BOLD); 
    attroff(COLOR_PAIR(1)); 
    refresh(); // maj du terminal
}
//Fonction pour initialiser ncurses
void init_View(){
 // NOTE: All ncurses operations (getch, mvaddch, refresh, etc.) must be done on the same thread.
    initscr(); /* Start curses mode */
    //raw(); /* Disable line buffering */
    cbreak();
    intrflush(stdscr, FALSE); /* No need to flush when intr key is pressed */
    keypad(stdscr, TRUE); /* Required in order to get events from keyboard */
    nodelay(stdscr, TRUE); /* Make getch non-blocking */
    noecho(); /* Don't echo() while we do getch (we will manually print characters when relevant) */
    curs_set(0); // curseur visible
    // Définir des paires de couleurs
    start_color(); // Activer les couleurs
    init_pair(1, COLOR_RED, COLOR_BLACK); // Joueur 1
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // Joueur 2
    init_pair(3, COLOR_BLUE, COLOR_BLACK); // Joueur 3
    init_pair(4, COLOR_YELLOW, COLOR_BLACK); // Joueur 4




}
