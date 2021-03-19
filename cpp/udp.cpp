#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <list>

#include "../headers/udp.h"
#include "../headers/types.h"

/**
 * Prototypes for internal functions
 * */

status_t receive(int sockfd, struct sockaddr_in server_addr, list<T3PResponse> *t3pResponseList);
status_t parse_response(string response, T3PResponse *t3pResponse);

/**
 * Functions
 * */

status_t send_discover_broadcast(list<T3PResponse> *t3pResponseList)
{
    // Define variables
    int broadcast_enable;
    int sockfd;
    struct timeval timeout;
    struct sockaddr_in server_addr;
    char *message = "DISCOVERBROADCAST \r\n \r\n";

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
        return ERROR_SOCKET_CREATION;
    
    // Configure broadcast enable
    broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0)
        return ERROR_SOCKET_CONFIGURATION;
    
    // Configure timeout
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        return ERROR_SOCKET_CONFIGURATION;

    // Set server_addr port, protocol and address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    // Send message
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        return ERROR_SENDING_MESSAGE;
    
    if (receive(sockfd, server_addr, t3pResponseList) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;
    
    close(sockfd);

    if ((*t3pResponseList).empty())
        return ERROR_NO_SERVERS_ONLINE;
    
    return STATUS_OK;
}


/**
 * Internal functions
 * */

status_t receive(int sockfd, struct sockaddr_in server_addr, list<T3PResponse> *t3pResponseList)
{
    // Declare variables
    int n;
    char c_response[BUFFER_SIZE] = {0};
    socklen_t len = sizeof(server_addr);
    string response;
    
    // Read until timeout or error. If parsing is ok, add the response to the list
    while ((n = recvfrom(sockfd, c_response, BUFFER_SIZE, 0, (struct sockaddr *) &server_addr, &len)) > 0)
    {
        T3PResponse t3pResponse;
        response = c_response;
        memset(c_response, 0, strlen(c_response));
        if ((parse_response(response, &t3pResponse)) == STATUS_OK)
            (*t3pResponseList).push_back(t3pResponse);
    }

    return STATUS_OK;
}

status_t parse_response(string response, T3PResponse *t3pResponse)
{
    // Declare variables
    list<string> tempStringList;
    size_t pos;
    int i;

    // Find terminating sequence
    if ((pos = response.rfind(" \r\n \r\n")) == string::npos)
        return ERROR_BAD_MESSAGE_FORMAT;

    // Erase last " \r\n"
    response.erase(pos+3);

    for(i=0; i<2; i++)
    {
        if ((pos = response.find('|')) == string::npos)
            return ERROR_BAD_MESSAGE_FORMAT;
        tempStringList.push_back(response.substr(0, pos));
        response.erase(0, pos+1);
    }

    while((pos = response.find(" \r\n")) != string::npos)
    {
        tempStringList.push_back(response.substr(0, pos));
        response.erase(0, pos+3);
    }

    (*t3pResponse).statusCode = tempStringList.front();
    tempStringList.pop_front();
    (*t3pResponse).statusMessage = tempStringList.front();
    tempStringList.pop_front();
    (*t3pResponse).dataList = tempStringList;

    return STATUS_OK;
}