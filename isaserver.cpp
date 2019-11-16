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

    if (argc > 3) {
        fprintf(stderr, "Too many arguments!\n");
        exit(EXIT_FAILURE);
    }

    if (port == nullptr) {
        fprintf(stderr, "Port number is the required argument!\n");
        exit(EXIT_FAILURE);
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
        currentMatch++;
    }
    return ok > 0;
}

bool isContentLengthOk(const std::string &buffer) {
    std::regex regex("Content-Length:[ \t]*[0-9]+\r\n");
    std::regex number("[0-9]+");
    std::smatch m2;
    std::smatch m;
    std::string content_length;

    regex_search(buffer, m, regex);
    for (auto x : m) {
        std::string found = x;
        regex_search(found, m2, number);
        for (auto y : m2) { content_length = y; }
        break;
    }

    std::string content = getContent(buffer);
    return content.size() == std::stoi(content_length) + 1;
}

int resolveCommand(int connfd, std::list<Board> &allBoards, Board &newBoard) {
    char buff[MAXSIZEOFREQUEST];
    int msg_size;
    int sendbytes;
    int code = 100;

    while ((msg_size = read(connfd, buff, MAXSIZEOFREQUEST)) > 0) {
        std::string buffer(buff);
        if (strncmp("GET", buff, 3) == 0) {
            code = getInfo(allBoards, connfd, buff);
            if (code == NOTFOUND) {
                std::string preparedRequest = prepareRespond(code);
                char request[preparedRequest.size() + 1];
                strcpy(request, preparedRequest.c_str());
                sendbytes = write(connfd, request, sizeof(request));
                if (sendbytes == -1) {
                    err(1, "write() failed.");
                }
            }
            return code;
        } else if (strncmp("POST", buff, 4) == 0) {
            int typeOfPost = post(buff, newBoard, allBoards);

            if (typeOfPost == CONFLICT) {
                code = 409;
            }
            if (typeOfPost == NEWBOARD) {
                code = POST_OK;
            }
            if (typeOfPost == UPGRADEBOARD) {
                code = upgradeBoardContent(buff, allBoards);
            }
            if (!isContentLengthOk(buffer)) {
                code = BADREQUEST;
            }
            std::string preparedRequest = prepareRespond(code);
            char request[preparedRequest.size() + 1];
            strcpy(request, preparedRequest.c_str());
            sendbytes = write(connfd, request, sizeof(request));
            if (typeOfPost == UPGRADEBOARD) {
                code = UPGRADEBOARD;
            }
        } else if (strncmp("DELETE", buff, 6) == 0) {
            code = deleteBoard(buff, allBoards);
            std::string preparedRequest = prepareRespond(code);
            char request[preparedRequest.size() + 1];
            strcpy(request, preparedRequest.c_str());
            sendbytes = write(connfd, request, sizeof(request));
        } else if (strncmp("PUT", buff, 3) == 0) {
            code = updateSpecificPost(buff, allBoards);
            if (!isContentLengthOk(buffer)) {
                code = BADREQUEST;
            }
            std::string preparedRequest = prepareRespond(code);
            char request[preparedRequest.size() + 1];
            strcpy(request, preparedRequest.c_str());
            sendbytes = write(connfd, request, sizeof(request));
        } else {
            code = NOTFOUND;
            std::string preparedRequest = prepareRespond(code);
            char request[preparedRequest.size() + 1];
            strcpy(request, preparedRequest.c_str());
            sendbytes = write(connfd, request, sizeof(request));
        }
        if (sendbytes == -1) {
            err(1, "write() failed.");
        }
    }
    return code;
}

std::string prepareRespond(int code) {
    time_t now = time(0);
    char *dt = ctime(&now);
    if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
    std::ostringstream stringStream;

    if (code == CONFLICT) {
        stringStream << "HTTP/1.1 " << code << " Conflict\r\nDate: " << dt << "\r\n\r\n";
    }
    if (code == NOTFOUND) {
        stringStream << "HTTP/1.1 " << code << " Not Found\r\nDate: " << dt << "\r\n\r\n";
    }
    if (code == POST_OK) {
        stringStream << "HTTP/1.1 " << code << " Created\r\nDate: " << dt << "\r\n\r\n";
    }
    if (code == OK) {
        stringStream << "HTTP/1.1 " << code << " OK\r\nDate: " << dt << "\r\n\r\n";
    }
    if (code == BADREQUEST) {
        stringStream << "HTTP/1.1 " << code << " Bad Request\r\nDate: " << dt << "\r\n\r\n";
    }
    std::string request = stringStream.str();
    return request;
}

int updateSpecificPost(char *buff, std::list<Board> &allBoards) {
    std::string command(buff);
    std::string content = getContent(command);
    std::vector<std::string> putCommandParts = getCommandPartsAsVector(command);
    std::string method = "PUT";
    long position = getPosition(putCommandParts, method);
    std::regex contentLength("Content-Length:[ \t]*[0-9]+\r\n");
    std::regex number("[0-9]+");
    std::string content_length;
    if (!isMatch(command, contentLength)) {
        return NOTFOUND;
    } else {
        std::smatch m2;
        std::smatch m;

        regex_search(command, m, contentLength);
        for (auto x : m) {
            std::string found = x;
            regex_search(found, m2, number);
            for (auto y : m2) { content_length = y; }
            break;
        }
    }

    if (content_length == "0") { return BADREQUEST; }
    if (position == 0) { return NOTFOUND; }
    if (content.empty()) { return BADREQUEST; }

    std::string name = putCommandParts.at(position + 2);
    std::regex put(R"(PUT[ \t]+\/board\/)" + name + R"(\/)" + putCommandParts.at(position + 3) +
                   R"([ \t]+HTTP\/1\.1[ \t]*\r\n)");
    if (!isMatch(command, put)) {
        return NOTFOUND;
    }

    int id = std::stoi(putCommandParts.at(position + 3));
    for (auto &i: allBoards) {
        if (putCommandParts.at(position + 2) == i.boardStructName) {
            if (id <= i.posts.size() && id >= 0) {
                findAndReplaceAll(i.posts.at(id - 1), i.posts.at(id - 1), content + "\n");
                return OK;
            }
        }
    }
    return NOTFOUND;
}

long getPosition(std::vector<std::string> &putCommandParts, const std::string &method) {
    long position = 0;

    std::vector<std::string>::iterator it;
    it = std::find(putCommandParts.begin(), putCommandParts.end(), method);
    if (it != putCommandParts.end()) {
        position = it - putCommandParts.begin() + 1;
    }
    return position;
}

int getInfo(std::list<Board> &allBoards, int connfd, char *buff) {
    int sendbytes;
    int code = NOTFOUND;
    time_t now = time(0);
    char *dt = ctime(&now);
    if (dt[strlen(dt) - 1] == '\n') { dt[strlen(dt) - 1] = '\0'; }
    std::string commands(buff);
    std::string commandContent = getContent(commands);
    std::vector<std::string> getCommandParts = getCommandPartsAsVector(commands);
    std::string method = "GET";
    long position = getPosition(getCommandParts, method);

    if (position == 0) { return NOTFOUND; }

    if (getCommandParts.at(position + 1) == "boards") {
        std::regex get1(R"(GET[ \t]+\/boards[ \t]+HTTP\/1\.1[ \t]*\r\n)");
        if (!isMatch(commands, get1)) {
            return NOTFOUND;
        }

        std::string content;
        for (auto &i: allBoards) {
            if (!i.boardStructName.empty()) {
                code = OK;
                content += i.boardStructName + "\n";
            }
        }
        if (code != OK) { return code; }

        findAndReplaceAll(content, "\\n", "\n");
        findAndReplaceAll(content, "\\n", "\n");
        findAndReplaceAll(content, "\\t", "\t");
        findAndReplaceAll(content, "\n\n", "\n");

        std::ostringstream stringStream;
        stringStream << "HTTP/1.1 " << code << " OK\r\nDate: " << dt
                     << "\r\nContent-Type: text/plain\r\nContent-Length: " << content.size() - 1 << "\r\n\r\n"
                     << content
                     << "\n";
        std::string copyOfStr = stringStream.str();
        if (copyOfStr.size() < MAXSIZEOFREQUEST - 20) {
            char request[copyOfStr.size() + 1];
            strcpy(request, copyOfStr.c_str());
            sendbytes = write(connfd, request, sizeof(request));
        } else {
            char request[MAXSIZEOFREQUEST];
            std::string truncated = copyOfStr.substr(0, MAXSIZEOFREQUEST - 20) + "\n";
            strcpy(request, truncated.c_str());
            sendbytes = write(connfd, request, sizeof(request));
        }
        if (sendbytes == -1) {
            err(1, "write() failed.");
        }
    }

    std::string name = getCommandParts.at(position + 2);
    std::regex get2(R"(GET[ \t]+\/board\/)" + name + R"([ \t]+HTTP\/1\.1[ \t]*\r\n)");
    if (!isMatch(commands, get2)) {
        return code;
    }

    for (auto &i: allBoards) {
        if (getCommandParts.at(position + 2) == i.boardStructName) {
            std::string contentOfPost = getContentOfPost(i);
            findAndReplaceAll(contentOfPost, "\\n", "\n");
            findAndReplaceAll(contentOfPost, "\\n", "\n");
            findAndReplaceAll(contentOfPost, "\\t", "\t");
            findAndReplaceAll(contentOfPost, "\n\n", "\n");

            code = OK;
            std::ostringstream stringStream;
            stringStream << "HTTP/1.1 " << code << " OK\r\nDate: " << dt
                         << "\r\nContent-Type: text/plain\r\nContent-Length: "
                         << contentOfPost.size() + i.boardStructName.size() + 2 << "\r\n\r\n" << "["
                         << i.boardStructName << "]" << "\n"
                         << contentOfPost << "\n";
            std::string copyOfStr = stringStream.str();
            if (copyOfStr.size() < MAXSIZEOFREQUEST - 20) {
                char request[copyOfStr.size() + 1];
                strcpy(request, copyOfStr.c_str());
                sendbytes = write(connfd, request, sizeof(request));
            } else {
                char request[MAXSIZEOFREQUEST];
                std::string truncated = copyOfStr.substr(0, MAXSIZEOFREQUEST - 20) + "\n";
                strcpy(request, truncated.c_str());
                sendbytes = write(connfd, request, sizeof(request));
            }
            if (sendbytes == -1) {
                err(1, "write() failed.");
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

    if (position == 0) { return NOTFOUND; }

    auto it = allBoards.begin();

    if (deleteCommandParts.at(position + 1) == "boards") {
        std::string name = deleteCommandParts.at(position + 2);
        std::regex delete1(R"(DELETE[ \t]+\/boards\/)" + name + R"([ \t]+HTTP\/1\.1[ \t]*\r\n)");
        if (!isMatch(command, delete1)) {
            return NOTFOUND;
        }

        int pos = 0;
        for (auto &i: allBoards) {
            if (deleteCommandParts.at(position + 2) == i.boardStructName) {
                advance(it, pos);
                allBoards.erase(it);
                return OK;
            }
            pos++;
        }
        return NOTFOUND;
    }

    int id = std::stoi(deleteCommandParts.at(position + 3));
    std::string name = deleteCommandParts.at(position + 2);
    std::regex delete2(R"(DELETE[ \t]+\/board\/)" + name + R"(\/)" + deleteCommandParts.at(position + 3) +
                       R"([ \t]+HTTP\/1\.1[ \t]*\r\n)");
    if (!isMatch(command, delete2)) {
        return NOTFOUND;
    }

    for (auto &i: allBoards) {
        if (deleteCommandParts.at(position + 2) == i.boardStructName) {
            if (id <= i.posts.size() && id >= 0) {
                i.posts.erase(i.posts.begin() + id - 1);
                return OK;
            }
        }
    }
    return NOTFOUND;
}

int upgradeBoardContent(char *buff, std::list<Board> &allBoards) {
    std::string command(buff);
    std::string content = getContent(command);
    std::vector<std::string> postCommandParts = getCommandPartsAsVector(command);
    std::string method = "POST";
    std::regex contentLength("Content-Length:[ \t]*[0-9]+\r\n");
    std::regex number("[0-9]+");

    long position = getPosition(postCommandParts, method);

    if (position == 0) { return NOTFOUND; }

    std::string name = postCommandParts.at(position + 2);
    std::regex delete1(R"(POST[ \t]+\/board\/)" + name + R"([ \t]+HTTP\/1\.1[ \t]*\r\n)");
    if (!isMatch(command, delete1)) {
        return NOTFOUND;
    }

    std::smatch m2;
    std::smatch m;
    std::string content_length;

    regex_search(command, m, contentLength);
    for (auto x : m) {
        std::string found = x;
        regex_search(found, m2, number);
        for (auto y : m2) { content_length = y; }
        break;
    }

    if (content_length == "0") { return BADREQUEST; }
    if (content.empty()) { return BADREQUEST; }

    for (auto &i: allBoards) {
        if (postCommandParts.at(position + 2) == i.boardStructName) {
            i.posts.push_back(content);
            return POST_OK;
        }
    }
    return NOTFOUND;
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

    if (position == 0) { return NOTFOUND; }

    if (postCommandParts.at(position + 1) == "boards" && isASCII(postCommandParts.at(position + 2))) {
        for (auto &i: allBoards) { if (postCommandParts.at(position + 2) == i.boardStructName) { return 409; }}
        std::string name = postCommandParts.at(position + 2);
        std::regex delete1(R"(POST[ \t]+\/boards\/)" + name + R"([ \t]+HTTP\/1\.1[ \t]*\r\n)");
        if (!isMatch(command, delete1)) {
            return NOTFOUND;
        }
        newBoard.boardStructName = postCommandParts.at(position + 2);
        return NEWBOARD;
    }

    if (postCommandParts.at(position + 1) == "boards" && !isASCII(postCommandParts.at(position + 2))) {
        return NOTFOUND;
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
    char buffer[MAXSIZEOFREQUEST];
    long p;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if ((bind(sockfd, (SA *) &servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }

    if ((listen(sockfd, MAXSIZEOFREQUEST)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    } else {
        printf("ISA server is listening...\n");
    }

    std::list<Board> arr;
    len = sizeof(cli);

    while (true) {
        connfd = accept(sockfd, (struct sockaddr *) &cli, (socklen_t *) &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }

        Board newBoard;
        int code = resolveCommand(connfd, arr, newBoard);
        if (code == POST_OK) {
            arr.push_back(newBoard);
        }

        close(connfd);
    }

    close(sockfd); // close an original server socket
    printf("* Closing the original socket\n");
    return 0;
}

int help() {
    printf("Help\n");

    exit(EXIT_SUCCESS);
}
