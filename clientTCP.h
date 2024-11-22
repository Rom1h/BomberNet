#ifndef CLIENTTCP_H
#define CLIENTTCP_H
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "formatage.h"

struct adresse_mdiff socket_client(char* port, char* address, int codereq,int * sock);
#endif
