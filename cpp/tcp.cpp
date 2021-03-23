#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <list>
#include <mutex>
#include <regex>
#include "../headers/tcp.h"
#include "../headers/types.h"

mutex msend;
bool connected = false;

using namespace std;

status_t send_tcp_message(int sockfd, const char *message);
status_t receive_tcp_message(int sockfd, T3PResponse *t3pResponse);
status_t parse_tcp_message(string response, T3PResponse *t3pResponse);

status_t login(Server server, string player_name, int *sockfd)
{
    struct sockaddr_in server_addr = {0};
    const char *c_ip = server.ip.c_str();
    const char *c_player_name = player_name.c_str();
    char message[40] = {0};
    T3PResponse t3pResponse;

    if ((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET_CREATION;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TCP_PORT);
    server_addr.sin_addr.s_addr = inet_addr(c_ip);

    if (connect((*sockfd), (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        close(*sockfd);
        return ERROR_CONNECTING;
    }

    sprintf(message, "LOGIN|%s \r\n \r\n", c_player_name);
    if (send_tcp_message((*sockfd), message) != STATUS_OK)
    {
        close(*sockfd);
        return ERROR_SENDING_MESSAGE;
    }

    if (receive_tcp_message((*sockfd), &t3pResponse) != STATUS_OK)
    {
        close(*sockfd);
        return ERROR_RECEIVING_MESSAGE;
    }

    if (t3pResponse.statusMessage != "OK")
        return ERROR_LOGIN;

    connected = true;
    return STATUS_OK;
}

status_t logout(int *sockfd)
{
    T3PResponse t3pResponse;
    const char *message = "LOGOUT \r\n \r\n";
    if (send_tcp_message(*sockfd, message) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;

    if (receive_tcp_message(*sockfd, &t3pResponse) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;

    if (t3pResponse.statusMessage != "OK")
        return ERROR_STATUS_MESSAGE;

    connected = false;
    close(*sockfd);
    return STATUS_OK;
}

status_t invite(int sockfd, string player_name)
{
    T3PResponse t3pResponse;
    const char *c_player_name = player_name.c_str();
    char message[BUFFER_SIZE];
    regex playerNameChecker("^[a-zA-Z]+$");

    // check if player's name is ok. Length must be between 3 and 20 chars, and it has to be alpha
    if (player_name.size() < 3 || player_name.size() > 20)
        return ERROR_BAD_PLAYER_NAME;
    if (!regex_match(player_name, playerNameChecker))
        return ERROR_BAD_PLAYER_NAME;

    // format string
    sprintf(message, "INVITE|%s \r\n \r\n", c_player_name);

    // send invite
    if (send_tcp_message(sockfd, message) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;

    if (receive_tcp_message(sockfd, &t3pResponse) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;

    if (t3pResponse.statusMessage != "OK")
        return ERROR_STATUS_MESSAGE;

    return STATUS_OK;
}

void heartbeat_thread(int sockfd)
{
    const char *message = "HEARTBEAT \r\n \r\n";
    while (connected == true)
    {
        send_tcp_message(sockfd, message);
        sleep(2);
    }   
}


/**
 * Internal functions
 * */

//Sends a message to a connected socket.
status_t send_tcp_message(int sockfd, const char *message)
{
    lock_guard<mutex> guard(msend);
    if (send(sockfd, message, strlen(message), 0) < 0)
        return ERROR_SENDING_MESSAGE;
    return STATUS_OK;
}

status_t receive_tcp_message(int sockfd, T3PResponse *t3pResponse)
{
    char response[BUFFER_SIZE];
    memset(response, 0, strlen(response));
    int bytes = read(sockfd, response, sizeof(response));
    if (bytes < 0)
        return ERROR_RECEIVING_MESSAGE;
    if (parse_tcp_message(string(response), t3pResponse) != STATUS_OK)
        return ERROR_BAD_MESSAGE_FORMAT;
    return STATUS_OK;
}

status_t parse_tcp_message(string response, T3PResponse *t3pResponse)
{
    // Format of TCP messages received in clients are:
    // STATUS_CODE|STATUS_MESSAGE \r\n \r\n

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