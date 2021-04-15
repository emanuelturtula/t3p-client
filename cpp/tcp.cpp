#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <list>
#include <mutex>
#include <regex>
#include <map>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/select.h>
#include "../headers/tcp.h"
#include "../headers/types.h"

mutex msend;
bool connected = false;

using namespace std;

void heartbeat_thread(context_t *context, int *sockfd)
{
    const char *message = "HEARTBEAT \r\n \r\n";
    while (*context != CLOSE_PROGRAM)
    {
        while((*context != MAIN_MENU) && (*context != SEARCH_LOCAL_SERVERS_MENU) && (*context != SEARCH_BY_IP_MENU) && (*context != CLOSE_PROGRAM))
        {
            if (connected == true)
            {
                if (send_tcp_message(*sockfd, message) != STATUS_OK)
                    return;
                sleep(2);
            }   
        }
        sleep(2);
    }
}

status_t get_connected_socket(string ip, int *sockfd)
{
    struct sockaddr_in server_addr = {0};
    struct timeval tv;
    fd_set fdset;
    
    *sockfd = -1;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET_CREATION;

    fcntl((*sockfd), F_SETFL, O_NONBLOCK);

    connect((*sockfd), (struct sockaddr *)&server_addr, sizeof(server_addr));

    FD_ZERO(&fdset);
    FD_SET((*sockfd), &fdset);
    tv.tv_sec = 10;             /* 10 second timeout */
    tv.tv_usec = 0;

    if (select((*sockfd) + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt((*sockfd), SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0)
        {
            fcntl((*sockfd), F_SETFL, 0);
            return STATUS_OK;
        }
    }

    close(*sockfd);
    return ERROR_CONNECTING;
}

status_t login(int sockfd, string player_name)
{
    status_t status;
    string message = "LOGIN|";
    T3PResponse t3pResponse;
    
    message += player_name + " \r\n \r\n";
    
    if ((status = send_tcp_message(sockfd, message.c_str())) != STATUS_OK)
        return status;

    if ((status = receive_tcp_message(sockfd, &t3pResponse)) != STATUS_OK)
        return status;

    if (t3pResponse.statusMessage != "OK")
        return ERROR_LOGIN;

    connected = true;
    return STATUS_OK;
}

status_t logout(int sockfd)
{
    status_t status;
    T3PResponse t3pResponse;
    const char *message = "LOGOUT \r\n \r\n";
    if ((status = send_tcp_message(sockfd, message)) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;

    if ((status = receive_tcp_message(sockfd, &t3pResponse)) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;
    
    if (t3pResponse.statusMessage != "OK")
        return T3PStatusCodeMapper[t3pResponse.statusCode];

    connected = false;
    return STATUS_OK;
}

status_t invite(int sockfd, string player_name, T3PCommand *t3pCommand)
{
    extern map<string, status_t> T3PStatusCodeMapper;
    status_t status;
    T3PResponse t3pResponse;
    string message = "INVITE|";
    regex playerNameChecker("^[a-zA-Z]+$");

    // check if player's name is ok. Length must be between 3 and 20 chars, and it has to be alpha
    if (player_name.size() < 3 || player_name.size() > 20)
        return ERROR_BAD_PLAYER_NAME;
    if (!regex_match(player_name, playerNameChecker))
        return ERROR_BAD_PLAYER_NAME;

    // format string
    message += player_name + " \r\n \r\n";
    
    // send invite
    if ((status = send_tcp_message(sockfd, message.c_str())) != STATUS_OK)
        return status;

    // receive the 200|ok
    if ((status = receive_tcp_message(sockfd, &t3pResponse)) != STATUS_OK)
        return status;

    if (t3pResponse.statusMessage != "OK")
        return T3PStatusCodeMapper[t3pResponse.statusCode];

    // Wait for response from server
    if ((status = receive_tcp_command(sockfd, t3pCommand)) != STATUS_OK)
        return status;

    return STATUS_OK;
}

status_t random_invite(int sockfd, T3PCommand *t3pCommand)
{
    extern map<string, status_t> T3PStatusCodeMapper;
    T3PResponse t3pResponse;
    status_t status;
    const char *message = "RANDOMINVITE \r\n \r\n";
    
    // Send the invite
    if ((status = send_tcp_message(sockfd, message)) != STATUS_OK)
        return status;

    // Read the 200 OK
    if ((status = receive_tcp_message(sockfd, &t3pResponse)) != STATUS_OK)
        return status;

    if (t3pResponse.statusMessage != "OK")
        return T3PStatusCodeMapper[t3pResponse.statusCode];

    // Wait for answer
    if ((status = receive_tcp_command(sockfd, t3pCommand)) != STATUS_OK)
        return status;
    
    return STATUS_OK;
}

status_t markslot(int sockfd, string slot)
{
    extern map<string, status_t> T3PStatusCodeMapper;
    status_t status;
    T3PResponse t3pResponse;
    T3PCommand t3pCommand;
    string message = "MARKSLOT|" + slot + " \r\n \r\n";
    if ((status = send_tcp_message(sockfd, message.c_str())) != STATUS_OK)
        return status;
    
    if ((status = receive_tcp_message(sockfd, &t3pResponse)) != STATUS_OK)
        return status;
    
    return T3PStatusCodeMapper[t3pResponse.statusCode];
}

status_t giveup(int sockfd)
{
    extern map<string, status_t> T3PStatusCodeMapper;
    status_t status;
    T3PResponse t3pResponse;
    const char* message = "GIVEUP \r\n \r\n";

    if ((status = send_tcp_message(sockfd, message)) != STATUS_OK)
        return status;

    if ((status = receive_tcp_message(sockfd, &t3pResponse)) != STATUS_OK)
        return status;

    return T3PStatusCodeMapper[t3pResponse.statusCode];
}


/**
 * Senders
 * */

status_t send_tcp_message(int sockfd, const char *message)
{
    lock_guard<mutex> guard(msend);
    if (send(sockfd, message, strlen(message), MSG_DONTWAIT) < 0)
        return ERROR_SENDING_MESSAGE;
    return STATUS_OK;
}

/**
 * Receivers
 * */

status_t receive_tcp_message(int sockfd, T3PResponse *t3pResponse)
{
    int read_bytes;
    status_t status;
    char response[BUFFER_SIZE] = {0};
    int strip_bytes = 0;
    int pos;

    memset(response, 0, strlen(response));
    string socket_message;

    if ((status = peek_tcp_buffer(sockfd, &read_bytes, &socket_message)) != STATUS_OK)
        return status;
    
    if ((pos = socket_message.find(" \r\n \r\n")) != string :: npos)
        strip_bytes = pos + strlen(" \r\n \r\n");
        
    int bytes = recv(sockfd, response, strip_bytes, 0);
    
    if (bytes < 0)
        return ERROR_RECEIVING_MESSAGE;
    if (parse_tcp_message(string(response), t3pResponse) != STATUS_OK)
        return ERROR_BAD_MESSAGE_FORMAT;
    return STATUS_OK;
}

status_t receive_tcp_command(int sockfd, T3PCommand *t3pCommand)
{
    status_t status;
    int read_bytes;
    int strip_bytes = 0;
    (*t3pCommand).clear();
    string socket_message;
    char message[BUFFER_SIZE] = {0};
    if ((status = peek_tcp_buffer(sockfd, &read_bytes, &socket_message)) != STATUS_OK)
        return status;

    int pos;
    if ((pos = socket_message.find(" \r\n \r\n")) != string :: npos)
        strip_bytes = pos + strlen(" \r\n \r\n");
        
    int bytes = recv(sockfd, message, strip_bytes, 0);

    if (bytes > 0)
    {
        if (parse_tcp_command(string(message), t3pCommand) != STATUS_OK)
            return ERROR_BAD_REQUEST;
        t3pCommand->isNewCommand = true;
    }
    return STATUS_OK;   
}

status_t peek_tcp_buffer(int sockfd, int *read_bytes, string *socket_message)
{
    char message[BUFFER_SIZE] = {0};
    if ((*read_bytes = recv(sockfd, message, sizeof(message), MSG_PEEK)) < 0)
        return ERROR_RECEIVING_MESSAGE;
    *socket_message = message;
    return STATUS_OK;
}

/**
 * Parsers
 * */

status_t parse_tcp_message(string response, T3PResponse *t3pResponse)
{
    size_t pos;
    string statusCode;
    string statusMessage;
    
    if ((pos = response.rfind(" \r\n \r\n")) == string::npos)
        return ERROR_BAD_MESSAGE_FORMAT;
    response.erase(pos+3);

    if ((pos = response.find("|")) == string::npos)
        return ERROR_BAD_MESSAGE_FORMAT;
    
    statusCode = response.substr(0, pos);
    response.erase(0, pos+1);

    if ((pos = response.find(" \r\n")) == string::npos)
        return ERROR_BAD_MESSAGE_FORMAT;
    
    statusMessage = response.substr(0, pos);

    (*t3pResponse).statusCode = statusCode;
    (*t3pResponse).statusMessage = statusMessage;

    return STATUS_OK;
}

status_t parse_tcp_command(string message, T3PCommand *t3pCommand)
{
    size_t pos;
    if ((pos = message.rfind(" \r\n \r\n")) == string::npos)
        return ERROR_BAD_REQUEST;
    
    // Strip last \r\n
    message.erase(pos+3);

    // If message contains "|", grab the first part as a command
    if ((pos = message.find("|")) != string::npos)
    {
        t3pCommand->command = message.substr(0, pos);
        message.erase(0, pos+1);
        while ((pos = message.find(" \r\n")) != string::npos)
        {
            t3pCommand->dataList.push_back(message.substr(0, pos));
            message.erase(0, pos+3);
        }
    }
    // else, the message should only contain the command
    else 
    {
        //I must have it because we checked at the beginning
        pos = message.find(" \r\n");
        t3pCommand->command = message.substr(0, pos);
    }
        
    return STATUS_OK;
}

tcpcommand_t parse_tcp_command(string socket_message, string *argument)
{
    extern map<string, tcpcommand_t> TCPCommandTranslator;
    string tcpCommand;
    if (socket_message.find("|") != string :: npos)
    {
        tcpCommand = socket_message.substr(0, socket_message.find("|"));
        socket_message.erase(0, socket_message.find("|")+1);
        (*argument) = socket_message.substr(0, socket_message.find(" \r\n"));
    }
    else
        tcpCommand = socket_message.substr(0, socket_message.find(" \r\n"));
    return TCPCommandTranslator[tcpCommand];
}

/**
 * Poll
 * */

status_t poll_event(int connectedSockfd, string *stdin_message, string *socket_message, int timeout)
{   
    status_t status;
    struct pollfd pfds[2]; // We monitor sockfd and stdin
    T3PCommand t3pCommand;
    int nevents = 0;

    if ((stdin_message == NULL) && (socket_message == NULL))
        return ERROR_NULL_POINTER;

    if (stdin_message != NULL)
    {
        *stdin_message = "";
        pfds[0].fd = 0;        // Stdin input
        pfds[0].events = POLLIN;    // Tell me when ready to read
        nevents++;
    }    
    
    if (socket_message != NULL)
    {
        *socket_message = "";
        pfds[1].fd = connectedSockfd;        // Sock input
        pfds[1].events = POLLIN;    // Tell me when ready to read
        nevents++;
    }  
   
    int num_events = poll(pfds, 2, timeout); // Wait until an event arrives
    

    if (pfds[0].revents & POLLIN)
        getline(cin, (*stdin_message));

    if (pfds[1].revents & POLLIN)
    {
        if ((status = receive_tcp_command(connectedSockfd, &t3pCommand)) != STATUS_OK)
            return status;

        *socket_message = t3pCommand.command;
        for (auto const& data : t3pCommand.dataList)
        {
            *socket_message += "|";
            *socket_message += data;
        }
    }

    return STATUS_OK;
}


