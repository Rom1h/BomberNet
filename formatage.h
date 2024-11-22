#ifndef FORMATAGE_H
#define FORMATAGE_H 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "board.h"
#include "type.h"
#include <assert.h>


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                   \
  ((byte)&0x80 ? '1' : '0'), ((byte)&0x40 ? '1' : '0'),            \
    ((byte)&0x20 ? '1' : '0'), ((byte)&0x10 ? '1' : '0'),        \
    ((byte)&0x08 ? '1' : '0'), ((byte)&0x04 ? '1' : '0'),        \
    ((byte)&0x02 ? '1' : '0'), ((byte)&0x01 ? '1' : '0')

struct adresse_mdiff{
    int entete[3];
    int portUDP;
    int portmdiff;
    char adresse[16];
};

char* format_entete(uint16_t code_req, uint16_t id, uint16_t eq);
int* deformat_entete(char* message);
void affiche_hexa(char* message, int t);
char* format_action(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t num, uint16_t action);
int* deformat_action(char* message);
char* format_tchat(uint16_t code_req, uint16_t id, uint16_t eq, uint8_t len, char* data);
int* deformat_entete_tchat(char* message);
char* format_adress_mdiff(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t portUDP, uint16_t portmdiff, char* adresse);
struct adresse_mdiff deformat_adress_mdiff(char* message);
char* format_fullgrid(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t num, board* plateau);
int* deformat_fullgrid_1(char* mess);
void deformat_fullgrid_2(char* mess, board* plateau);
char* format_partialgrid(uint16_t code_req, uint16_t id, uint16_t eq, uint16_t num, uint8_t nb, case_t* changements);
int* deformat_partialgrid_1(char* mess);
void deformat_partialgrid_2(char* mess,board * board,int nb);
#endif