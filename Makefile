SERVEUR_TARGET = serveur 
CLIENT_TARGET = client

CC = gcc

CFLAGS = -Wall -g 

NCURSE=-lncurses
CLIENT_SRCS =  formatage.c gameView.c gameUpdate.c board.c ClientUDP.c mainClient.c clientTCP.c chat.c
SERVEUR_SRCS =  formatage.c gameView.c gameUpdate.c board.c threads_serveur.c serveur.c chat.c


#client: $(CLIENT_TARGET) 
#serveur: $(SERVEUR_TARGET)

all : $(CLIENT_TARGET) $(SERVEUR_TARGET)

$(CLIENT_TARGET) : $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRCS) $(NCURSE)

$(SERVEUR_TARGET) : $(SERVEUR_SRCS)
	$(CC) $(CFLAGS) -o $(SERVEUR_TARGET) $(SERVEUR_SRCS) $(NCURSE)

clean : 
	rm -f $(CLIENT_TARGET) $(SERVEUR_TARGET)