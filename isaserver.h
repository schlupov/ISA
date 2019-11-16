//
// Created by silvie on 10/1/19.
//

#ifndef ISASERVER_ISASERVER_H
#define ISASERVER_ISASERVER_H
#include <netdb.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>
#include <arpa/inet.h>
#include <vector>
#include <list>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <regex>
#define MAXSIZEOFREQUEST 8192
#define SA struct sockaddr
#define NEWBOARD 0
#define UPGRADEBOARD 1
#define CONFLICT 409
#define NOTFOUND 404
#define OK 200
#define POST_OK 201
#define BADREQUEST 400
#include <utility>

struct Board {
    std::string boardStructName;
    int id;
    std::vector<std::string> posts;
};

int help();
int startServer(int port);
int resolveCommand(int connfd, std::list<Board>& allBoards, Board &newBoard);
int post(char *buff, Board &newBoard, std::list<Board> &allBoards);
void tokenize(std::string const &str, char delimiter, std::vector<std::string> &out);
bool isASCII(const std::string& s);
std::string getContent(const std::string &command);
int upgradeBoardContent(char *buff, std::list<Board>& allBoards);
std::vector<std::string> getCommandPartsAsVector(std::string &command);
int getInfo(std::list<Board> &allBoards, int connfd, char *buff);
int deleteBoard(char *buff, std::list<Board>& allBoards);
std::string getContentOfPost(Board &board);
void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr);
bool isMatch(std::string str, std::regex reg);
int updateSpecificPost(char *buff, std::list<Board> &allBoards);
long getPosition(std::vector<std::string> &putCommandParts, const std::string &method);
std::string prepareRespond(int code);
bool isContentLengthOk(const std::string& buffer);
#endif //ISASERVER_ISASERVER_H
