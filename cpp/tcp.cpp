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

status_t random_invite(int sockfd)
{
    T3PResponse t3pResponse;
    const char *message = "RANDOMINVITE \r\n \r\n";
    if (send_tcp_message(sockfd, message) != STATUS_OK)
        return ERROR_SENDING_MESSAGE;

    if (receive_tcp_message(sockfd, &t3pResponse) != STATUS_OK)
        return ERROR_RECEIVING_MESSAGE;

    if (t3pResponse.statusMessage != "OK")
        return ERROR_STATUS_MESSAGE;
    
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
status_t invitation_response(int sockfd,bool response){

    status_t status;
    string message;
    if(response == true)
        message = "ACCEPT \r\n \r\n";
    else
        message = "DECLINE \r\n \r\n";

    return send_tcp_message(sockfd, message.c_str()); //returns status_t of function send_tcp_message()
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

status_t receive_tcp_command(int sockfd, T3PCommand *t3pCommand, context_t context)
{
    status_t status;
    (*t3pCommand).clear();
    char message[BUFFER_SIZE] = {0};
    int bytes = recv(sockfd, message, sizeof(message), 0);
    if (bytes > 0)
    {
        if (parse_tcp_command(string(message), t3pCommand) != STATUS_OK)
            return ERROR_BAD_REQUEST;
        if ((status = check_tcp_command(*t3pCommand, context)) != STATUS_OK)
            return status;
        t3pCommand->isNewCommand = true;
    }
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

status_t check_tcp_command(T3PCommand t3pCommand, context_t context)
{
    status_t status;
    string playerName;
    list<string> availablePlayers;
    tcpcommand_t command;
    // First we need to check if the incomming command is present
    // in a list of possible commands.
    if (find(TCPCommands.begin(), TCPCommands.end(), t3pCommand.command) == TCPCommands.end())
        return ERROR_BAD_REQUEST;

    // Translate the command to a tcpcommand type to ease the later code
    command = TCPCommandTranslator[t3pCommand.command];
    // Now that we know that the command actually exists, we need 
    // to check based on the context if it is valid.
    switch(context)
    {
        case SOCKET_CONNECTED:
            switch(command)
            {
                case LOGIN:
                    playerName = t3pCommand.dataList.front();
                    if ((status = checkPlayerName(playerName)) != STATUS_OK)
                        return status;
                    if (checkPlayerIsOnline(playerName))
                        return ERROR_NAME_TAKEN;
                    break;
                default:
                    return ERROR_COMMAND_OUT_OF_CONTEXT;
            }            
            break;
        case LOBBY:
            switch(command)
            {
                case HEARTBEAT:
                    break;
                case INVITE:
                    playerName = t3pCommand.dataList.front();
                    if ((status = checkPlayerName(playerName)) != STATUS_OK)
                        return status;
                    if (!checkPlayerIsOnline(playerName))
                        return ERROR_PLAYER_NOT_FOUND;
                    if (!checkPlayerIsAvailable(playerName))
                        return ERROR_PLAYER_OCCUPIED;
                    break;
                case RANDOMINVITE:
                    availablePlayers = mainDatabase.getAvailablePlayers();
                    if (availablePlayers.empty())
                        return INFO_NO_PLAYERS_AVAILABLE;
                    break;
                case LOGOUT:
                    break;
                default:
                    return ERROR_COMMAND_OUT_OF_CONTEXT;
            }
            break;
        case WAITING_OTHER_PLAYER_RESPONSE:
            switch(command)
            {
                case HEARTBEAT:
                    break;
                default: 
                    return ERROR_COMMAND_OUT_OF_CONTEXT;
            }
            break;
        case WAITING_RESPONSE:
            switch(command)
            {
                case HEARTBEAT:
                    break;
                case ACCEPT:
                    break;
                case DECLINE:
                    break;
                default:
                    return ERROR_COMMAND_OUT_OF_CONTEXT;
            }
            break;
        case READY_TO_PLAY:
            switch(command)
            {
                case HEARTBEAT:
                    break;
                default:
                    return ERROR_COMMAND_OUT_OF_CONTEXT;
            }
            break;
        case MATCH:
            switch(command)
            {
                case HEARTBEAT:
                    break;
                case MARKSLOT:
                    break;
                case GIVEUP:
                    break;
                case OK:
                    break;
                case BAD:
                    break;
                default:
                    return ERROR_COMMAND_OUT_OF_CONTEXT;
            }
            break;
    }
    return STATUS_OK;
}


/*need to check if the player is in the correct context first
If the function return STATUS_OK we should change the context so the 
player would leave the game*/
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
