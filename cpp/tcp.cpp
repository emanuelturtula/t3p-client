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
#include <iostream>
#include "../headers/tcp.h"
#include "../headers/types.h"

mutex msend;
bool connected = false;

using namespace std;


/**
 * Methods for T3PCommand
 * */
T3PServerMessages :: T3PServerMessages()
{
    this->name = "";
    this->dataList.clear();
}

string T3PServerMessages::getName(){
    return this->name;
}

void T3PServerMessages :: clear()
{
    this->name = "";
    this->dataList.clear();
}

void T3PServerMessages::addData(string data){
    this->dataList.push_back(data);
}

status_t T3PServerMessages::setName(string name){

    if( name.compare("INVITATIONTIMEOUT") == 0 ||
        name.compare("INVITEFROM") == 0 ||
        name.compare("TURNPLAY") == 0 ||
        name.compare("TURNWAIT") == 0 ||
        name.compare("MATCHEND") == 0){
        this->name = name;
    }else{
        return ERROR_BAD_REQUEST;
    }

    return STATUS_OK;
}

status_t T3PServerMessages::parse_buffer(string dataStream){
    size_t pos;
    status_t status;

    this->clear();


    if ((pos = dataStream.rfind(" \r\n \r\n")) == string::npos)
    return ERROR_BAD_REQUEST;

    // Strip last \r\n
    dataStream.erase(pos+3);

    // If message contains "|", grab the first part as a command
    if ((pos = dataStream.find("|")) != string::npos)
    {

        if (( status = this->setName(dataStream.substr(0, pos))) != STATUS_OK)
        dataStream.erase(0, pos+1);
        while ((pos = dataStream.find("|")) != string::npos || (pos = dataStream.find(" \r\n")) != string::npos)
        {
            this->addData(dataStream.substr(0, pos));
            dataStream.erase(0, pos+1);
        }
    }
    // else, the message should only contain the command
    else 
    {
        //I must have it because we checked at the beginning
        pos = dataStream.find(" \r\n");
        this->setName(dataStream.substr(0, pos));
    }
        
    return STATUS_OK;

}

/**
 * END--------Methods for T3PCommand
 * */

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
    const char *message = "LOGOUT \r\n \r\n";
    if (send_tcp_message(*sockfd, message) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;

    connected = false;
    close(*sockfd);
    return STATUS_OK;
}

status_t invite(int sockfd, string player_name, bool *response)
{
    T3PResponse t3pResponse;
    T3PCommand t3pCommand;
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

    // receive the 200|ok
    if (receive_tcp_message(sockfd, &t3pResponse) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;

    if (t3pResponse.statusMessage != "OK")
        return ERROR_STATUS_MESSAGE;

    if (receive_tcp_command(sockfd, &t3pCommand) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;

    *response = false;
    if (t3pCommand.command == "ACCEPT")
        *response = true;

    return STATUS_OK;
}

status_t random_invite(int sockfd, T3PCommand *t3pCommand)
{
    T3PResponse t3pResponse;
    const char *message = "RANDOMINVITE \r\n \r\n";
    
    // Send the invite
    if (send_tcp_message(sockfd, message) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;

    // Read the 200 OK
    if (receive_tcp_message(sockfd, &t3pResponse) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;

    if (t3pResponse.statusCode == "101")
        return INFO_NO_PLAYERS_AVAILABLE;

    if (t3pResponse.statusMessage != "OK")
        return ERROR_STATUS_MESSAGE;

    // Wait for answer
    if (receive_tcp_command(sockfd, t3pCommand) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;
    
    return STATUS_OK;
}

status_t wait_invitation_response(int sockfd, bool *accept)
{
    char c_response[BUFFER_SIZE];
    size_t pos;
    string response;
    
    memset(c_response, 0, strlen(c_response));
    
    int bytes = recv(sockfd, c_response, sizeof(c_response), 0);
    if (bytes < 0)
        return ERROR_RECEIVING_MESSAGE;

    response = c_response;
    if (response.rfind(" \r\n \r\n") == string :: npos)
        return ERROR_BAD_MESSAGE_FORMAT;
    pos = response.find(" \r\n");
    response = response.substr(0, pos);
    if (response == "ACCEPT")
        *accept = true;
    else if (response == "DECLINE")
        *accept = false;
    else
        return ERROR_BAD_MESSAGE_FORMAT;
    
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

/* According to variable "response" it sends an ACCEPT or DECLINE command.
    true = ACCEPT
    false = DECLINE*/
status_t send_invitation_response(int sockfd, bool response){

    status_t status;
    string message;
    if(response == true)
        message = "ACCEPT \r\n \r\n";
    else
        message = "DECLINE \r\n \r\n";

    return send_tcp_message(sockfd, message.c_str()); //returns status_t of function send_tcp_message()
}

status_t markslot(int sockfd, string slot)
{
    T3PResponse t3pResponse;
    T3PCommand t3pCommand;
    string message = "MARKSLOT|" + slot + " \r\n \r\n";
    if (send_tcp_message(sockfd, message.c_str()) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;
    
    if (receive_tcp_message(sockfd, &t3pResponse) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;
    
    return T3PStatusCodeMapper[t3pResponse.statusCode];
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
    
    int bytes = recv(sockfd, response, sizeof(response), 0);
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
    if ((pos = socket_message.find_first_of(" \r\n \r\n")) != string :: npos)
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

status_t giveup(int sockfd)
{
    T3PResponse t3pResponse;
   // char message[BUFFER_SIZE];

    // format string
    const char* message = "GIVEUP \r\n \r\n";

    // send giveup
    if (send_tcp_message(sockfd, message) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;


    if (receive_tcp_message(sockfd, &t3pResponse) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;

    if (t3pResponse.statusMessage != "OK")
        return ERROR_STATUS_MESSAGE;

    return STATUS_OK;
}

// General poll function. Polls for sockfd for 5 seconds and copies the result of data readed in "data_stream"
status_t poll_tcp_message(int sockfd, string *data_stream){

    char c_response[BUFFER_SIZE];
    string response;

    memset(c_response, 0, strlen(c_response));

    struct pollfd pfds[1]; // We monitor sockfd

    pfds[0].fd = sockfd;        // Sock input
    pfds[0].events = POLLIN;    // Tell me when ready to read

    int num_events = poll(pfds, 1, 5000); // We pool for 5 sec.

    if (num_events == 0) {
        (*data_stream) == ""; // We do not return anything
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if (pfds[0].revents & POLLIN){ // Sockfd has been written

            int bytes = recv(sockfd, c_response, sizeof(c_response), 0);

            if (bytes < 0)
                return ERROR_RECEIVING_MESSAGE;

            (*data_stream) = c_response;
            return STATUS_OK;

        }else {
            return ERROR_UNEXPECTED_EVENT_POLL_TCP_INVITATION_FROM;
        }
    }
    return STATUS_OK;
}

status_t poll_event(int connectedSockfd, string *stdin_message, string *socket_message)
{   
    struct pollfd pfds[2]; // We monitor sockfd and stdin


    *stdin_message = "";
    *socket_message = "";

    pfds[0].fd = 0;        // Stdin input
    pfds[0].events = POLLIN;    // Tell me when ready to read

    pfds[1].fd = connectedSockfd;        // Sock input
    pfds[1].events = POLLIN;    // Tell me when ready to read

    int num_events = poll(pfds, 2, -1); // Wait until an event arrives

    int pollin_happened = pfds[0].revents & POLLIN;

    if (pfds[0].revents & POLLIN)
        getline(cin, (*stdin_message));

    if (pfds[1].revents & POLLIN)
    {
        char c_response[BUFFER_SIZE];
        memset(c_response, 0, strlen(c_response));
        int bytes = recv(connectedSockfd, c_response, sizeof(c_response), 0);
        if (bytes < 0)
            return ERROR_RECEIVING_MESSAGE;
        (*socket_message) = c_response;
    }

    return STATUS_OK;
}

tcpcommand_t parse_tcp_command(string socket_message, string *argument)
{
    map<string, tcpcommand_t> TCPCommandTranslator = {
        {"INVITEFROM", INVITEFROM},
        {"INVITATIONTIMEOUT", INVITATIONTIMEOUT},
        {"ACCEPT", ACCEPT},
        {"DECLINE", DECLINE},
        {"TURNPLAY", TURNPLAY},
        {"TURNWAIT", TURNWAIT},
        {"MATCHEND", MATCHEND}
    };

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
