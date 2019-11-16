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
#include <regex>
#include <sstream>
#include <sys/types.h>
#define SA struct sockaddr
#define window 8192

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
int connect(char *port, char *host, char *command, char *contentForPost, bool verbose);
char *convertCommandtoHttpRequest(char *command, char *fullHttpCommand);
char* createHttpCommand(std::string whichHttpCommand, const std::string& basicString, char *fullHttpCommand);
int httpMethod(int sockfd, char *method, const char *port, const char *host, char *string, bool verbose);
std::string getContent(const std::string &command);
bool isMatch(std::string str, std::regex reg);
bool checkCommandLineArguments(const std::string& command);
std::string getHeaders(const std::string &command);
int checkHttpReturnCode(std::string headers);
std::string getCode(std::string headers, std::regex regex);
#endif
