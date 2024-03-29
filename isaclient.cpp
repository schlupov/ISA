#include "isaclient.h"


int communicateWithServer(int sockfd, char *method, const char *port, const char *host, char *data) {
    char fromServer[MAXSIZEOFREQUEST];
    std::ostringstream stringStream;
    std::string strData(data);
    std::string request;
    int sendbytes;

    if (strncmp(method, "POST", 4) == 0) {
        stringStream << method << " HTTP/1.1\r\nHost: " << host << ":" << port
                     << "\r\nContent-Type: text/plain\r\nContent-Length: " << strData.size()-1 << "\r\n\r\n" << strData
                     << "\n";
        request = stringStream.str();
        if (request.size() >= MAXSIZEOFREQUEST) {
            printf("Request is too long, please make it shorter.\n");
            exit(EXIT_FAILURE);
        }
    }
    if (strncmp(method, "GET", 3) == 0) {
        stringStream << method << " HTTP/1.1\r\nHost: " << host << ":" << port << "\r\n\r\n";
        request = stringStream.str();
    }
    if (strncmp(method, "DELETE", 6) == 0) {
        stringStream << method << " HTTP/1.1\r\nHost: " << host << ":" << port << "\r\n\r\n";
        request = stringStream.str();
    }
    if (strncmp(method, "PUT", 3) == 0) {
        stringStream << method << " HTTP/1.1\r\nHost: " << host << ":" << port
                     << "\r\nContent-Type: text/plain\r\nContent-Length: " << strData.size()-1 << "\r\n\r\n" << strData
                     << "\n";
        request = stringStream.str();
        if (request.size() >= MAXSIZEOFREQUEST) {
            printf("Request is too long, please make it shorter.\n");
            exit(EXIT_FAILURE);
        }
    }

    char requestChar[request.size() + 1];
    strcpy(requestChar, request.c_str());
    sendbytes = write(sockfd, requestChar, sizeof(requestChar));
    if (sendbytes == -1) {
        err(1, "write() failed.");
    }
    int msg_size;
    std::string stringFromServer;
    bzero(fromServer, sizeof(fromServer));

    while ((msg_size = read(sockfd, fromServer, MAXSIZEOFREQUEST)) > 0) {
        std::string tmp2(fromServer);
        stringFromServer += tmp2;
        bzero(fromServer, sizeof(fromServer));
    }

    int code = 0;
    std::string contentStringFromServer = getContent(stringFromServer);
    std::string headersStringFromServer = getHeaders(stringFromServer);
    code = checkHttpReturnCode(headersStringFromServer);
    fprintf(stderr, "%s", headersStringFromServer.c_str());
    printf("%s", contentStringFromServer.c_str());

    if (code > 201) {
        return -1;
    }
    return 0;
}

int checkHttpReturnCode(std::string headers) {
    std::regex code(R"((200|201|400|404|409))");
    std::string strCode = getCode(headers, code);
    int intCode = 400;
    try {
        intCode = std::stoi(strCode);
    }
    catch (std::invalid_argument const &e) {
        return intCode;
    }
    catch (std::out_of_range const &e) {
        return intCode;
    }
    return intCode;
}

std::string getCode(std::string headers, std::regex regex) {
    std::sregex_iterator currentMatch(headers.begin(), headers.end(), regex);
    std::sregex_iterator lastMatch;

    while (currentMatch != lastMatch) {
        std::smatch match = *currentMatch;
        return match.str();
    }
    return " ";
}

std::string getHeaders(const std::string &command) {
    std::string headers;
    int beginOfContent = 0;
    for (unsigned int i = 0; i < command.length(); i++) {
        if (command[i] == '\n' && command[i + 1] == '\r') {
            beginOfContent = i + 3;
        }
    }

    for (unsigned int i = 0; i < beginOfContent; i++) {
        headers += command[i];
    }
    return headers;
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

void tokenize(std::string const &str, const char delimiter, std::vector<std::string> &out) {
    std::stringstream ss(str);

    std::string s;
    while (std::getline(ss, s, delimiter)) {
        out.push_back(s);
    }
}

char *convertCommandtoHttpRequest(char *command, char *fullHttpCommand) {
    std::string token(command);
    const char space = ' ';
    std::vector<std::string> commands;
    tokenize(command, space, commands);

    States state = start;
    std::string front;
    int i = 0;

    while (i < commands.size()) {
        front = commands[i];
        i++;

        switch (state) {
            case start:
                if (front == "boards") {
                    std::string tmp = "GET /boards";
                    char cstr[tmp.size() + 1];
                    strcpy(cstr, tmp.c_str());
                    strcpy(fullHttpCommand, cstr);
                    return fullHttpCommand;
                } else if (front == "board") {
                    state = board;
                } else if (front == "item") {
                    state = item;
                } else {
                    fprintf(stderr, "Wrong argument: %s\n", front.c_str());
                    exit(EXIT_FAILURE);
                }
                break;
            case item:
                if (front == "add") {
                    state = item_add;
                } else if (front == "delete") {
                    state = item_delete_command;
                } else if (front == "update") {
                    state = update;
                } else {
                    fprintf(stderr, "Wrong argument: %s\n", front.c_str());
                    exit(EXIT_FAILURE);
                }
                break;
            case board:
                if (front == "add") {
                    state = add;
                } else if (front == "delete") {
                    state = delete_commmand;
                } else if (front == "list") {
                    state = list;
                } else {
                    fprintf(stderr, "Wrong argument 2: %s\n", front.c_str());
                    exit(EXIT_FAILURE);
                }
                break;
            case item_delete_command:
                if (i == 4) {
                    char cstr[front.size() + 1];
                    strcpy(cstr, front.c_str());
                    strcat(fullHttpCommand, "/");
                    strcat(fullHttpCommand, cstr);
                } else {
                    fullHttpCommand = createHttpCommand("DELETE /board/", front, fullHttpCommand);
                }
                break;
            case item_add:
                fullHttpCommand = createHttpCommand("POST /board/", front, fullHttpCommand);
                return fullHttpCommand;
            case update:
                if (i == 4) {
                    char cstr[front.size() + 1];
                    strcpy(cstr, front.c_str());
                    strcat(fullHttpCommand, "/");
                    strcat(fullHttpCommand, cstr);
                    return fullHttpCommand;
                } else {
                    fullHttpCommand = createHttpCommand("PUT /board/", front, fullHttpCommand);
                }
                break;
            case add:
                fullHttpCommand = createHttpCommand("POST /boards/", front, fullHttpCommand);
                break;
            case delete_commmand:
                fullHttpCommand = createHttpCommand("DELETE /boards/", front, fullHttpCommand);
                break;
            case list:
                fullHttpCommand = createHttpCommand("GET /board/", front, fullHttpCommand);
                break;
        }
    }

    return fullHttpCommand;
}

char *createHttpCommand(std::string whichHttpCommand, const std::string &front, char *fullHttpCommand) {
    whichHttpCommand += front;
    char cstr[whichHttpCommand.size() + 1];
    strcpy(cstr, whichHttpCommand.c_str());
    strcpy(fullHttpCommand, cstr);
    return fullHttpCommand;
}

bool isMatch(std::string str, std::regex reg) {
    std::sregex_iterator currentMatch(str.begin(), str.end(), reg);
    std::sregex_iterator lastMatch;

    while (currentMatch != lastMatch) {
        std::smatch match = *currentMatch;
        return str == match.str();
    }
    return false;
}

int main(int argc, char *argv[]) {
    int opt;
    char *port = nullptr;
    char *host = nullptr;

    while ((opt = getopt(argc, argv, ":p:H:h")) != -1) {
        switch (opt) {
            case 'p':
                port = optarg;
                break;
            case 'H':
                host = optarg;
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
        fprintf(stderr, "Port number is the required argument!\n");
        return EXIT_FAILURE;
    }

    if (host == nullptr) {
        printf("Host address is the required argument!\n");
        return EXIT_FAILURE;
    }

    unsigned long len = 0;
    int tmpArgumentsCounter = argc - 5;
    for (int i = tmpArgumentsCounter; i > 0; --i) {
        len = len + strlen(argv[argc - i]);
    }
    char *command = (char *) malloc(len * 2);
    char *contentForPost = (char *) malloc(strlen(argv[argc - 1]) * 2);

    for (int i = tmpArgumentsCounter; i > 0; --i) {
        strcat(command, argv[argc - i]);
        if (i != 1) {
            strcat(command, " ");
        } else {
            strcat(contentForPost, argv[argc - i]);
        }
    }
    std::string strCommand(command);
    bool everythingOk = checkCommandLineArguments(strCommand, argc);
    if (!everythingOk) {
        fprintf(stderr, "Bad program arguments, use -h\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints{}, *infoptr;
    hints.ai_family = AF_INET;

    int result = getaddrinfo(host, NULL, &hints, &infoptr);
    if (result) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        exit(1);
    }

    struct addrinfo *p;
    char hostname[256];

    for (p = infoptr; p != nullptr; p = p->ai_next) {
        getnameinfo(p->ai_addr, p->ai_addrlen, hostname, sizeof(hostname), nullptr, 0, NI_NUMERICHOST);
        break;
    }

    freeaddrinfo(infoptr);

    char *fullHttpCommand = (char *) malloc(len * 2);
    fullHttpCommand = convertCommandtoHttpRequest(command, fullHttpCommand);
    int code = connect(port, hostname, fullHttpCommand, contentForPost);
    free(command);
    free(fullHttpCommand);

    if (code == -1) { exit(EXIT_FAILURE); }
    exit(0);
}

int help() {
    printf("Help\n");

    exit(EXIT_SUCCESS);
}

bool checkCommandLineArguments(const std::string &command, int argc) {
    if (command.find("item") != std::string::npos && command.find("update") != std::string::npos) {
        std::regex update(R"((item update [a-zA-Z0-9]+ [0-9]+ [\x00-\x7F]+))");
        if (argc != 10) { return false; }
        if (!isMatch(command, update)) { return false; }
    } else if (command.find("item") != std::string::npos && command.find("add") != std::string::npos) {
        std::regex update(R"((item add [a-zA-Z0-9]+ [\x00-\x7F]+))");
        if (argc != 9) { return false; }
        if (!isMatch(command, update)) { return false; }
    } else if (command.find("item") != std::string::npos && command.find("delete") != std::string::npos) {
        std::regex update(R"((item delete [a-zA-Z0-9]+ [0-9]+))");
        if (argc != 9) { return false; }
        if (!isMatch(command, update)) { return false; }
    } else if (command.find("board") != std::string::npos && command.find("list") != std::string::npos) {
        std::regex update(R"((board list [a-zA-Z0-9]+))");
        if (argc != 8) { return false; }
        if (!isMatch(command, update)) { return false; }
    } else if (command.find("board") != std::string::npos && command.find("delete") != std::string::npos) {
        std::regex update(R"((board delete [a-zA-Z0-9]+))");
        if (argc != 8) { return false; }
        if (!isMatch(command, update)) { return false; }
    } else if (command.find("boards") != std::string::npos) {
        std::regex update(R"((boards))");
        if (argc != 6) { return false; }
        if (!isMatch(command, update)) { return false; }
    } else if (command.find("board") != std::string::npos && command.find("add") != std::string::npos) {
        std::regex update(R"((board add [a-zA-Z0-9]+))");
        if (argc != 8) { return false; }
        if (!isMatch(command, update)) { return false; }
    } else {
        return false;
    }
    return true;
}

int connect(char *port, char *host, char *command, char *contentForPost) {
    int sockfd;
    struct sockaddr_in servaddr{};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);
    servaddr.sin_port = htons(atoi(port));

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }

    int code = communicateWithServer(sockfd, command, port, host, contentForPost);

    close(sockfd);

    return code;
}
