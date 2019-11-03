#include "isaserver.h"


int main(int argc, char *argv[]) {
    int opt;
    char *port = nullptr;

    while ((opt = getopt(argc, argv, ":p:h")) != -1) {
        switch (opt) {
            case 'p':
                port = optarg;
                break;
            case 'h':
                help();
                break;
            default:
                help();
                break;
        }
    }

    if (port == nullptr) {
        printf("Port number is required argument!\n");
        return EXIT_FAILURE;
    }

    startServer(atoi(port));

    return 0;
}

bool isMatch(std::string str, std::regex reg) {
    std::sregex_iterator currentMatch(str.begin(), str.end(), reg);
    std::sregex_iterator lastMatch;
    int ok = 0;

    while (currentMatch != lastMatch) {
        ok++;
        std::smatch match = *currentMatch;
        std::cout << match.str() << "\n";
        currentMatch++;
    }
    return ok > 0;
}

int resolveCommand(int connfd, std::list<Board> &allBoards, Board &newBoard) {
    char buff[LISTENQ];
    char returncode[MAX];
    int msg_size;
    int code = 100;

    while ((msg_size = read(connfd, buff, sizeof(buff))) > 0) {
        printf("%s\n", buff);
        if (strncmp("GET", buff, 3) == 0) {
            code = getInfo(allBoards, connfd, buff);
            if (code == 404) {
                time_t now = time(0);
                char *dt = ctime(&now);
                if (dt[strlen(dt)-1] == '\n') { dt[strlen(dt)-1] = '\0'; }
                char request[LISTENQ] = {0};
                bzero(request, sizeof(request));
                sprintf(request, "HTTP/1.1 %d Not Found\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n",
                        code, dt);
                int sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                } else if (sendbytes != sizeof(request)) {
                    err(1, "write(): buffer written partially");
                }
            }
            return code;
        } else if (strncmp("POST", buff, 4) == 0) {
            int typeOfPost = post(buff, newBoard, allBoards);

            if (typeOfPost == 409) {
                code = 409;
                time_t now = time(0);
                char *dt = ctime(&now);
                if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
                char request[LISTENQ] = {0};
                bzero(request, sizeof(request));
                sprintf(request, "HTTP/1.1 %d Conflict\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n",
                        code, dt);
                int sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                } else if (sendbytes != sizeof(request)) {
                    err(1, "write(): buffer written partially");
                }
            }
            if (typeOfPost == NEWBOARD) {
                code = 201;
                time_t now = time(0);
                char *dt = ctime(&now);
                if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
                char request[LISTENQ] = {0};
                bzero(request, sizeof(request));
                sprintf(request, "HTTP/1.1 %d OK\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n", code, dt);
                int sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                } else if (sendbytes != sizeof(request)) {
                    err(1, "write(): buffer written partially");
                }
            }
            if (typeOfPost == UPGRADEBOARD) {
                code = upgradeBoardContent(buff, allBoards);
                if (code == 404) {
                    time_t now = time(0);
                    char *dt = ctime(&now);
                    if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
                    char request[LISTENQ] = {0};
                    bzero(request, sizeof(request));
                    sprintf(request, "HTTP/1.1 %d Not Found\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n",
                            code, dt);
                    int sendbytes = write(connfd, request, sizeof(request));
                    if (sendbytes == -1) {
                        err(1, "write() failed.");
                    } else if (sendbytes != sizeof(request)) {
                        err(1, "write(): buffer written partially");
                    }
                }
                if (code == 201) {
                    time_t now = time(0);
                    char *dt = ctime(&now);
                    if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
                    char request[LISTENQ] = {0};
                    bzero(request, sizeof(request));
                    sprintf(request, "HTTP/1.1 %d OK\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n", code, dt);
                    int sendbytes = write(connfd, request, sizeof(request));
                    if (sendbytes == -1) {
                        err(1, "write() failed.");
                    } else if (sendbytes != sizeof(request)) {
                        err(1, "write(): buffer written partially");
                    }
                }
                code = UPGRADEBOARD;
            }
        } else if (strncmp("DELETE", buff, 6) == 0) {
            code = deleteBoard(buff, allBoards);
            if (code == 404) {
                time_t now = time(0);
                char *dt = ctime(&now);
                if (dt[strlen(dt)-1] == '\n') { dt[strlen(dt)-1] = '\0'; }
                char request[LISTENQ] = {0};
                bzero(request, sizeof(request));
                sprintf(request, "HTTP/1.1 %d Not Found\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n",
                        code, dt);
                int sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                } else if (sendbytes != sizeof(request)) {
                    err(1, "write(): buffer written partially");
                }
            }
            if (code == 200) {
                time_t now = time(0);
                char *dt = ctime(&now);
                if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
                char request[LISTENQ] = {0};
                bzero(request, sizeof(request));
                sprintf(request, "HTTP/1.1 %d OK\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n", code, dt);
                int sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                } else if (sendbytes != sizeof(request)) {
                    err(1, "write(): buffer written partially");
                }
            }
        } else if (strncmp("PUT", buff, 3) == 0) {
            code = updateSpecificPost(buff, allBoards);
            if (code == 201) {
                time_t now = time(0);
                char *dt = ctime(&now);
                if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
                char request[LISTENQ] = {0};
                bzero(request, sizeof(request));
                sprintf(request, "HTTP/1.1 %d OK\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n", code, dt);
                int sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                } else if (sendbytes != sizeof(request)) {
                    err(1, "write(): buffer written partially");
                }
            }
            if (code == 404) {
                time_t now = time(0);
                char *dt = ctime(&now);
                if (dt[strlen(dt)-1] == '\n') { dt[strlen(dt)-1] = '\0'; }
                char request[LISTENQ] = {0};
                bzero(request, sizeof(request));
                sprintf(request, "HTTP/1.1 %d Not Found\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n",
                        code, dt);
                int sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                } else if (sendbytes != sizeof(request)) {
                    err(1, "write(): buffer written partially");
                }
            }
        } else {
            time_t now = time(0);
            char *dt = ctime(&now);
            if (dt[strlen(dt)-1] == '\n') { dt[strlen(dt)-1] = '\0'; }
            char request[LISTENQ] = {0};
            bzero(request, sizeof(request));
            sprintf(request, "HTTP/1.1 %d Not Found\r\nDate: %s\r\nContent-Type: text/plain\r\n\r\n",
                    code, dt);
            int sendbytes = write(connfd, request, sizeof(request));
            if (sendbytes == -1) {
                err(1, "write() failed.");
            } else if (sendbytes != sizeof(request)) {
                err(1, "write(): buffer written partially");
            }
        }
    }
    return code;
}

int updateSpecificPost(char *buff, std::list<Board> &allBoards) {
    std::string command(buff);
    std::string content = getContent(command);
    std::vector<std::string> putCommandParts = getCommandPartsAsVector(command);
    std::string method = "PUT";
    long position = getPosition(putCommandParts, method);
    std::regex contentLength("Content-Length: [0-9]+");
    std::regex number("[0-9]+");
    std::string numberStr;
    if (!isMatch(command, contentLength)) {
        return 404;
    }
    else {
        std::sregex_iterator currentMatch(command.begin(), command.end(), contentLength);
        std::sregex_iterator lastMatch;

        while (currentMatch != lastMatch) {
            std::smatch match = *currentMatch;
            std::sregex_iterator currentMatchStr(match.str().begin(), match.str().end(), number);
            std::sregex_iterator lastMatchStr;
            while (currentMatchStr != lastMatchStr) {
                std::smatch matchNumber = *currentMatchStr;
                numberStr = matchNumber.str();
                break;
            }
            break;
        }
    }

    if (numberStr == "0") { return 400; }

    if (position == 0) { return 404; }

    if (content.empty()) { return 400; }

    std::string name = putCommandParts.at(position + 2);
    std::regex put(R"(PUT \/board\/)" + name + R"(\/)" + putCommandParts.at(position + 3) + R"( HTTP\/1\.1)");
    if (!isMatch(command, put)) {
        return 404;
    }

    int id = std::stoi(putCommandParts.at(position + 3));
    for (auto &i: allBoards) {
        if (putCommandParts.at(position + 2) == i.boardStructName) {
            if (id <= i.posts.size() && id >= 0) {
                findAndReplaceAll(i.posts.at(id - 1), i.posts.at(id - 1), content + "\n");
                return 201;
            }
        }
    }
    return 404;
}

long getPosition(std::vector<std::string> &putCommandParts, const std::string &method) {
    long position = 0;

    std::vector<std::string>::iterator it;
    it = std::find (putCommandParts.begin(), putCommandParts.end(), method);
    if (it != putCommandParts.end()) {
        position = it - putCommandParts.begin() + 1;
    }
    return position;
}

int getInfo(std::list<Board> &allBoards, int connfd, char *buff) {
    char request[LISTENQ] = {0};
    int sendbytes;
    int code = 404;
    time_t now = time(0);
    char *dt = ctime(&now);
    if (dt[strlen(dt)-1] == '\n') { dt[strlen(dt)-1] = '\0'; }
    std::string commands(buff);
    std::string commandContent = getContent(commands);
    std::vector<std::string> getCommandParts = getCommandPartsAsVector(commands);
    std::string method = "GET";
    long position = getPosition(getCommandParts, method);

    if (position == 0) { return 404; }

    if (getCommandParts.at(position + 1) == "boards") {
        std::regex get1(R"(GET \/boards HTTP\/1\.1)");
        if (!isMatch(commands, get1)) {
            return 404;
        }

        std::string content;
        for (auto &i: allBoards) {
            if (!i.boardStructName.empty()) {
                code = 200;
                content += i.boardStructName + "\n";
            }
        }
        if (code != 200) { return code; }

        findAndReplaceAll(content, "\\n", "\n");
        findAndReplaceAll(content, "\\n", "\n");
        findAndReplaceAll(content, "\n\n", "\n");

        char cstr[content.size() + 1];
        strcpy(cstr, content.c_str());
        bzero(request, sizeof(request));
        sprintf(request, "HTTP/1.1 %d OK\r\nDate: %s\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n%s\n",
                code, dt, strlen(cstr)-1, cstr);
        sendbytes = write(connfd, request, sizeof(request));
        if (sendbytes == -1) {
            err(1, "write() failed.");
        } else if (sendbytes != sizeof(request)) {
            err(1, "write(): buffer written partially");
        }
    }

    std::string name = getCommandParts.at(position + 2);
    std::regex get2(R"(GET \/board\/)" + name + R"( HTTP\/1\.1)");
    if (!isMatch(commands, get2)) {
        return code;
    }

    for (auto &i: allBoards) {
        if (getCommandParts.at(position + 2) == i.boardStructName) {
            char name[i.boardStructName.size() + 1];
            strcpy(name, i.boardStructName.c_str());

            std::string tmp = getContentOfPost(i);
            findAndReplaceAll(tmp, "\\n", "\n");
            findAndReplaceAll(tmp, "\\n", "\n");
            findAndReplaceAll(tmp, "\n\n", "\n");

            code = 200;
            char contentOfPost[tmp.size() + 1];
            strcpy(contentOfPost, tmp.c_str());
            bzero(request, sizeof(request));
            sprintf(request,
                    "HTTP/1.1 %d OK\r\nDate: %s\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n[%s]\n%s\n",
                    code, dt, strlen(contentOfPost) + strlen(name)+2, name, contentOfPost);
            sendbytes = write(connfd, request, sizeof(request));
            if (sendbytes == -1) {
                err(1, "write() failed.");
            } else if (sendbytes != sizeof(request)) {
                err(1, "write(): buffer written partially");
            }
            return code;
        }
    }
    return code;
}

void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr) {
    size_t pos = data.find(toSearch);
    while (pos != std::string::npos) {
        data.replace(pos, toSearch.size(), replaceStr);
        pos = data.find(toSearch, pos + replaceStr.size());
    }
}

std::string getContentOfPost(Board &board) {
    std::string tmp;
    int position = 1;
    for (auto &i: board.posts) {
        tmp += std::to_string(position) + ". ";
        for (char content : i) {
            tmp += content;
        }
        position++;
    }
    return tmp;
}

int deleteBoard(char *buff, std::list<Board> &allBoards) {
    std::string command(buff);
    std::string content = getContent(command);
    std::vector<std::string> deleteCommandParts = getCommandPartsAsVector(command);
    std::string method = "DELETE";
    long position = getPosition(deleteCommandParts, method);

    if (position == 0) { return 404; }

    auto it = allBoards.begin();

    if (deleteCommandParts.at(position + 1) == "boards") {
        std::string name = deleteCommandParts.at(position + 2);
        std::regex delete1(R"(DELETE \/boards\/)" + name + R"( HTTP\/1\.1)");
        if (!isMatch(command, delete1)) {
            return 404;
        }

        int pos= 0;
        for (auto &i: allBoards) {
            if (deleteCommandParts.at(position + 2) == i.boardStructName) {
                advance(it, pos);
                allBoards.erase(it);
                return 200;
            }
            pos++;
        }
        return 404;
    }

    int id = std::stoi(deleteCommandParts.at(position + 3));
    std::string name = deleteCommandParts.at(position + 2);
    std::regex delete2(R"(DELETE \/board\/)" + name + R"(\/)" + deleteCommandParts.at(position + 3) + R"( HTTP\/1\.1)");
    if (!isMatch(command, delete2)) {
        return 404;
    }

    for (auto &i: allBoards) {
        if (deleteCommandParts.at(position + 2) == i.boardStructName) {
            if (id <= i.posts.size() && id >= 0) {
                i.posts.erase(i.posts.begin() + id - 1);
                return 200;
            }
        }
    }
    return 404;
}

int upgradeBoardContent(char *buff, std::list<Board> &allBoards) {
    std::string command(buff);
    std::string content = getContent(command);
    std::vector<std::string> postCommandParts = getCommandPartsAsVector(command);
    std::string method = "POST";
    long position = getPosition(postCommandParts, method);

    if (position == 0) { return 404; }

    std::string name = postCommandParts.at(position + 2);
    std::regex delete1(R"(POST \/board\/)" + name + R"( HTTP\/1\.1)");
    if (!isMatch(command, delete1)) {
        return 404;
    }

    for (auto &i: allBoards) {
        if (postCommandParts.at(position + 2) == i.boardStructName) {
            i.posts.push_back(content);
            return 201;
        }
    }
    return 404;
}

void tokenize(std::string const &str, const char delimiter, std::vector<std::string> &out) {
    std::stringstream ss(str);

    std::string s;
    while (std::getline(ss, s, delimiter)) {
        out.push_back(s);
    }
}

int post(char *buff, Board &newBoard, std::list<Board> &allBoards) {
    std::string command(buff);
    std::vector<std::string> postCommandParts = getCommandPartsAsVector(command);
    std::string method = "POST";
    long position = getPosition(postCommandParts, method);

    if (position == 0) { return 404; }

    if (postCommandParts.at(position + 1) == "boards" && isASCII(postCommandParts.at(position + 2))) {
        for (auto &i: allBoards) { if (postCommandParts.at(position + 2) == i.boardStructName) { return 409; }}
        std::string name = postCommandParts.at(position + 2);
        std::regex delete1(R"(POST \/boards\/)" + name + R"( HTTP\/1\.1)");
        if (!isMatch(command, delete1)) {
            return 404;
        }
        newBoard.boardStructName = postCommandParts.at(position + 2);
        return NEWBOARD;
    }

    if (postCommandParts.at(position + 1) == "boards" && !isASCII(postCommandParts.at(position + 2))) {
        return 404;
    }

    return UPGRADEBOARD;
}

std::vector<std::string> getCommandPartsAsVector(std::string &command) {
    const char delimiter = '/';
    const char space = ' ';
    std::vector<std::string> postLine;
    std::vector<std::string> postCommandParts;
    std::string boardName;
    std::string board;
    tokenize(command, space, postLine);
    for (auto &i : postLine) {
        tokenize(i, delimiter, postCommandParts);
    }
    return postCommandParts;
}

std::string getContent(const std::string &command) {
    std::string content;
    int beginOfContent = 0;
    for (unsigned int i = 0; i < command.length(); i++) {
        if (command[i] == '\n' && command[i + 1] == '\r') {
            beginOfContent = i + 3;
        }
    }

    for (unsigned int i = beginOfContent; i < command.length(); i++) {
        content += command[i];
    }
    return content;
}

bool isASCII(const std::string &s) {
    return !std::any_of(s.begin(), s.end(), [](char c) {
        return static_cast<unsigned char>(c) > 127;
    });
}

int startServer(int port) {
    int sockfd, connfd, len, i;
    struct sockaddr_in servaddr{}, cli{};
    pid_t pid;
    int msg_size;
    char buffer[LISTENQ];
    long p;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else {
        printf("Socket successfully created..\n");
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else {
        printf("Socket successfully binded..\n");
    }

    if ((listen(sockfd, LISTENQ)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("Server listening..\n");
    }

    std::list<Board> arr;
    len = sizeof(cli);

    while (true) {
        connfd = accept(sockfd, (struct sockaddr *) &cli, (socklen_t *) &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        } else {
            printf("server accept the client...\n");
        }

        Board newBoard;
        int code = resolveCommand(connfd, arr, newBoard);
        if (code == 201) {
            arr.push_back(newBoard);
        }

        close(connfd);
        printf("* Closing newsock\n");
    }

    close(sockfd); // close an original server socket
    printf("* Closing the original socket\n");
    return 0;
}

int help() {
    printf("Help\n");

    exit(EXIT_SUCCESS);
}