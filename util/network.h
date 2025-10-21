#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

extern int OpenServer(int portnr);
extern int CloseServer(int socketfd);

extern int ConnectToServer(char * Server, int Port);
extern int DisconnectFromServer(int socketfd);

