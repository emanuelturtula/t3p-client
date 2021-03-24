#include "server.h"

using namespace std;

int respond(int connfd, string buffer);

void tcp_thread()
{
    int sockfd;
    int connfd;
    int response;
    socklen_t len;
    struct sockaddr_in server;
    struct sockaddr_in client;
    char buffer[TCP_BUFFER_SIZE];

    server.sin_family = AF_INET;
    server.sin_port = htons(TCP_PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        write_stderr("TCP THREAD - ERROR CREATING SOCKET");
        return;
    }

    if ((bind(sockfd, (struct sockaddr *)&server, sizeof(server))) != 0)
    {
        close(sockfd);
        write_stderr("TCP THREAD - ERROR BINDING");
        return;
    }

    if ((listen(sockfd, 5)) != 0)
    {
        close(sockfd);
        write_stderr("TCP THREAD - LISTENING FAILED");
        return;
    }

    write_stdout("TCP THREAD - LISTENING");

    if ((connfd = accept(sockfd, (struct sockaddr *)&client, &len)) < 0)
    {
        close(sockfd);
        write_stdout("TCP THREAD - ACCEPTING CONNECTION FAILED");
        return;
    }
    
    write_stdout("TCP THREAD - CONNECTION SUCCESSFUL");

    while(1)
    {
        memset(buffer, 0, strlen(buffer));
        if (read(connfd, buffer, sizeof(buffer)) < 0)
        {
            close(connfd);
            close(sockfd);
            write_stderr("TCP THREAD - ERROR READING MESSAGE");
            return;
        }
        if ((response = respond(connfd, string(buffer))) < 0)
        {
            close(connfd);
            close(sockfd);
            write_stderr("TCP THREAD - ERROR RESPONDING");
            return;
        }
        else if (response == 1)
            return;
    }
}

int respond(int connfd, string buffer)
{
    const char *login_response = "200|OK \r\n \r\n";
    const char *logout_response = "200|OK \r\n \r\n";
    if (buffer == "HEARTBEAT \r\n \r\n")
        write_stdout("TCP THREAD - Received HEARTBEAT.");
    else if (buffer == "LOGIN|edturtu \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received LOGIN.");
        if (write(connfd, login_response, strlen(login_response)) < 0)
            return -1;
    }
    else if (buffer == "LOGOUT \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received LOGOUT.");
        if (write(connfd, logout_response, strlen(logout_response)) < 0)
            return -1;
        return 1;
    }
    else 
    {
        write_stderr("TCP THREAD - Unrecognized command.");
    }
    return 0;
}