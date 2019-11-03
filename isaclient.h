#ifndef XCHLUP08_2_ISACLIENT_H
#define XCHLUP08_2_ISACLIENT_H
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#define SA struct sockaddr
#define window 1024

typedef enum {
    start,
    board,
    item,
    add,
    delete_commmand,
    list,
    update,
    item_add,
    item_delete_command
} States;

int help();
int connect(char *port, char *host, char *command, char *contentForPost);
char *convertCommandtoHttpRequest(char *command, char *fullHttpCommand);
char* createHttpCommand(std::string whichHttpCommand, const std::string& basicString, char *fullHttpCommand);
void http_method(int sockfd, char *method, const char *port, const char *host, char *string);
std::string getContent(const std::string &command);
#endif
