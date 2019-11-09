#include "isaclient.h"

void http_method(int sockfd, char *method, const char *port, const char *host, char *data) {
    char fromServer[window];
    char request[window] = {0};
    bzero(request, sizeof(request));

    if (strncmp(method, "POST", 4) == 0) {
        sprintf(request, "%s HTTP/1.1\r\nHost: %s:%s\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s\n",
                method, host, port, strlen(data), data);
    }
    if (strncmp(method, "GET", 3) == 0) {
        sprintf(request, "%s HTTP/1.1\r\nHost: %s:%s\r\nContent-Type: text/plain\r\n\r\n",
                method, host, port);
    }
    if (strncmp(method, "DELETE", 6) == 0) {
        sprintf(request, "%s HTTP/1.1\r\nHost: %s:%s\r\nContent-Type: text/plain\r\n\r\n",
                method, host, port);
    }
    if (strncmp(method, "PUT", 3) == 0) {
        sprintf(request, "%s HTTP/1.1\r\nHost: %s:%s\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s\n",
                method, host, port, strlen(data), data);
    }

    write(sockfd, request, sizeof(request));
    bzero(fromServer, sizeof(fromServer));
    read(sockfd, fromServer, sizeof(fromServer));
    std::string stringFromServer(fromServer);
    //std::string contentStringFromServer = getContent(stringFromServer);
    //std::cout << contentStringFromServer;
    std::cout << stringFromServer;
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
        printf("Port number is a required argument!\n");
        return EXIT_FAILURE;
    }

    if (host == nullptr) {
        printf("Host address is a required argument!\n");
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

    if (((strcmp(argv[argc - 4], "add") == 0) || (strcmp(argv[argc - 4], "delete") == 0)) && tmpArgumentsCounter != 4) {
        fprintf(stderr, "Wrong number of arguments for command add or delete\n");
        exit(EXIT_FAILURE);
    }

    if ((strcmp(argv[argc - 4], "update") == 0) && tmpArgumentsCounter != 5) {
        fprintf(stderr, "Wrong number of arguments for command update\n");
        exit(EXIT_FAILURE);
    }

    if ((strcmp(argv[argc - 4], "board") == 0) && tmpArgumentsCounter != 3) {
        fprintf(stderr, "Wrong number of arguments with board\n");
        exit(EXIT_FAILURE);
    }

    if ((strcmp(argv[argc - 4], "boards") == 0) && tmpArgumentsCounter != 1) {
        fprintf(stderr, "Wrong number of arguments with boards\n");
        exit(EXIT_FAILURE);
    }

    char *fullHttpCommand = (char *) malloc(len * 2);
    fullHttpCommand = convertCommandtoHttpRequest(command, fullHttpCommand);
    connect(port, host, fullHttpCommand, contentForPost);
    free(command);
    free(fullHttpCommand);

    return EXIT_SUCCESS;
}

int help() {
    printf("Help\n");

    exit(EXIT_SUCCESS);
}

int connect(char *port, char *host, char *command, char *contentForPost) {
    int sockfd;
    struct sockaddr_in servaddr{};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    } else {
        //printf("Socket successfully created..\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);
    servaddr.sin_port = htons(atoi(port));

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    } else {
        //printf("connected to the server..\n");
    }

    http_method(sockfd, command, port, host, contentForPost);

    close(sockfd);

    return 0;
}
