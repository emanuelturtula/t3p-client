#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <list>

#include "../headers/udp.h"
#include "../headers/types.h"

extern map<string, int> config;

status_t receive(int sockfd, struct sockaddr_in server_addr, list<T3PResponse> *t3pResponseList);
status_t parse_response(string response, T3PResponse *t3pResponse);


status_t send_discover_broadcast(list<T3PResponse> *t3pResponseList)
{
    status_t status;
    int broadcast_enable;
    int sockfd;
    struct timeval timeout;
    struct sockaddr_in server_addr;
    const char *message = "DISCOVERBROADCAST \r\n \r\n";

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
        return ERROR_SOCKET_CREATION;
    
    // Configure broadcast enable
    broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0)
    {
        close(sockfd);
        return ERROR_SOCKET_CONFIGURATION;
    }
        
    // Configure timeout
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        close(sockfd);
        return ERROR_SOCKET_CONFIGURATION;
    }    

    // Set server_addr port, protocol and address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config["UDP_PORT"]);
    server_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    // Send message
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sockfd);
        return ERROR_SENDING_MESSAGE;
    }    
        
    (*t3pResponseList).clear();
    if ((status = receive(sockfd, server_addr, t3pResponseList)) != STATUS_OK)
    {
        close(sockfd);
        return status;
    }
        
    close(sockfd);
    
    return STATUS_OK;
}

status_t send_discover(string ip, T3PResponse *t3pResponse)
{
    // Define variables
    status_t status;
    int sockfd;
    struct timeval timeout;
    struct sockaddr_in server_addr;
    char message[1024];
    const char *c_ip = ip.c_str();
    list<T3PResponse> t3pResponseList;
    sprintf(message, "DISCOVER|%s \r\n \r\n", c_ip);

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
        return ERROR_SOCKET_CREATION;
        
    // Configure timeout
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        return ERROR_SOCKET_CONFIGURATION;

    // Set server_addr port, protocol and address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config["UDP_PORT"]);
    server_addr.sin_addr.s_addr = inet_addr(c_ip);
    
    // Send message
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sockfd);
        return ERROR_SENDING_MESSAGE;
    }    
    
    t3pResponseList.clear();    
    if ((status = receive(sockfd, server_addr, &t3pResponseList)) != STATUS_OK)
    {
        close(sockfd);
        return status;
    }    
        
    close(sockfd);
    *t3pResponse = t3pResponseList.front();
    
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
    status_t status;
    
    // Read until timeout or error. If parsing is ok, add the response to the list
    while ((n = recvfrom(sockfd, c_response, BUFFER_SIZE, 0, (struct sockaddr *) &server_addr, &len)) > 0)
    {
        T3PResponse t3pResponse;
        response = c_response;
        memset(c_response, 0, strlen(c_response));
        if ((status = parse_response(response, &t3pResponse)) != STATUS_OK)
            return status;
        (*t3pResponseList).push_back(t3pResponse);
    }
    if ((n < 0) && ((*t3pResponseList).empty()))
        return ERROR_NO_SERVERS_ONLINE;

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
        return ERROR_BAD_REQUEST;

    // Erase last " \r\n"
    response.erase(pos+3);

    for(i=0; i<2; i++)
    {
        if ((pos = response.find('|')) == string::npos)
            return ERROR_BAD_REQUEST;
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