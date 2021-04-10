#include "server.h"

using namespace std;

int respond(int connfd, string buffer);

void tcp_thread()
{
    bool connected = false;
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

    while (1)
    {
        sleep(2);
        connected = false;
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
        
        connected = true;
        write_stdout("TCP THREAD - CONNECTION SUCCESSFUL");

        while(connected == true)
        {
            memset(buffer, 0, strlen(buffer));
            if (recv(connfd, buffer, sizeof(buffer),0) < 0)
                write_stderr("TCP THREAD - ERROR READING MESSAGE");
            if ((response = respond(connfd, string(buffer))) < 0)
                write_stderr("TCP THREAD - ERROR RESPONDING");
            else if (response == 1)
            {
                close(connfd);
                write_stdout("TCP THREAD - CLOSING CONNECTION");
                connected = false;
            }

        }
    }
    
}

int respond(int connfd, string buffer)
{
    const char *login_response = "200|OK \r\n \r\n";
    const char *logout_response = "200|OK \r\n \r\n";
    const char *invite_response = "200|OK \r\n \r\n";
    const char *random_invite_response = "200|OK \r\n \r\n";
    const char *accept_invite_response = "ACCEPT \r\n \r\n";
    const char *decline_invite_response = "DECLINE \r\n \r\n";
    if (buffer == "HEARTBEAT \r\n \r\n")
        write_stdout("TCP THREAD - Received HEARTBEAT.");
    else if (buffer == "LOGIN|edturtu \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received LOGIN.");
        if (send(connfd, login_response, strlen(login_response),0) < 0)
            return -1;
    }
    else if (buffer == "LOGOUT \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received LOGOUT.");
        if (send(connfd, logout_response, strlen(logout_response),0) < 0)
            return -1;
        return 1;
    }
    else if (buffer == "INVITE|edturtu \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received INVITE from edturtu.");
        if (send(connfd, invite_response, strlen(invite_response),0) < 0)
            return -1;
        write_stdout("TCP THREAD - Writed accept_invite_response.");
        if (send(connfd, accept_invite_response, strlen(accept_invite_response),0) < 0)
            return -1;
    }
    else if (buffer == "RANDOMINVITE \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received RANDOMINVITE.");
        if (send(connfd, random_invite_response, strlen(random_invite_response),0) < 0)
            return -1;
            
        write_stdout("TCP THREAD - Writed decline_invite_response.");
        if (send(connfd, decline_invite_response, strlen(decline_invite_response),0) < 0)
            return -1;
    }
    else if (buffer == "ACCEPT \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received ACCEPT.");
        return 1;
    }
    else if (buffer == "DECLINE \r\n \r\n")
    {
        write_stdout("TCP THREAD - Received DECLINE.");
        return 1;
    }
    else 
    {
        write_stderr("TCP THREAD - Unrecognized command.");
        write_stdout("TCP THREAD - COMMAND RECEIVED: " + buffer);
        return 1;
    }
    return 0;
}