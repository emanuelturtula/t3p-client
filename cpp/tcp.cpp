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


status_t poll_tcp_message_invitationtimeout_or_stdin(int sockfd, context_t *context){

    status_t status;
    struct pollfd pfds[2]; // We monitor 2 things: sockfd and stdin

    pfds[0].fd = 0;          // Standard input
    pfds[0].events = POLLIN; // Tell me when ready to read

    pfds[1].fd = sockfd; // Some socket descriptor
    pfds[1].events = POLLIN;  // Tell me when ready to read

    int num_events = poll(pfds, 2, -1); //We pool forever

    if (num_events == 0) {
        // This case should be impossible but is added just in case
        return ERROR_POLL_DETECTED_0_EVENTS;
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if (pfds[0].revents & POLLIN){ // stdin has been written. We return to go to "getline()"
            return STATUS_OK;
        }

        else if (pfds[1].revents & POLLIN){// sockfd has been written. 
        // We read socket for message. This is a case where the server send us a message when we are
        // deciding something from the UI. This case only can happen on:
        // INVITATIONTIMEOUT
        // INVITATIONFROM
        // TIMEOUTWIN/TIMEOUTLOSE
            return receive_invitation_from_timeout(sockfd, context);
        }else {
            return ERROR_UNEXPECTED_EVENT_POLL_TCP_INVITATION_FROM;
        }
    }

    return STATUS_OK;
}

status_t receive_invitation_from_timeout(int sockfd,context_t *context){

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
    if (response == "INVITATIONTIMEOUT"){
        *context = LOBBY_MENU;
        std::cout << "You didn't respond invitation: TIMEOUT. Returning to LOBBY" << endl;
        return STATUS_OK;
    }
    else
        return ERROR_BAD_MESSAGE_FORMAT;
    
    return STATUS_OK;
}

// We poll to recive a interaction from the clientplayer, or if server is sending us an INVITEFROM
// COPIED FROM PREVIOUS FUNCTION
status_t poll_tcp_message_invitationfrom_or_stdin(int sockfd, context_t *context, string *invitationhost){

    status_t status;
    struct pollfd pfds[2]; // We monitor 2 things: sockfd and stdin

    pfds[0].fd = stdin->_fileno; // Standard input
    pfds[0].events = POLLIN; // Tell me when ready to read

    pfds[1].fd = sockfd; // Some socket descriptor
    pfds[1].events = POLLIN;  // Tell me when ready to read

    int num_events = poll(pfds, 2, -1); //We pool forever

    if (num_events == 0) {
        // This case should be impossible but is added just in case
        return ERROR_POLL_DETECTED_0_EVENTS;
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if (pfds[0].revents & POLLIN){ // stdin has been written. We return to go to "getline()"
            return STATUS_OK;
        }

        else if (pfds[1].revents & POLLIN){// sockfd has been written. 
        // We read socket for message. This is a case where the server send us a message when we are
        // deciding something from the UI. This case only can happen on:
        // INVITATIONFROM
            return receive_invitation_from(sockfd, context, invitationhost);
        }else {
            return ERROR_UNEXPECTED_EVENT_POLL_TCP_INVITATION_FROM;
        }
    }

    return STATUS_OK;
}


status_t receive_invitation_from(int sockfd,context_t *context, string *invitationhost){

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

    // we separete INVITEFROM|<playerName>
    if ((pos = response.find("|")) != string :: npos){

        if(response.substr(0,pos) != "INVITEFROM")
            return ERROR_NOT_RECIVIED_INVTEFROM;
        
        *invitationhost = response.substr(pos+1);
        (*context) = INVITATIONFROM;
    }
    else
        return ERROR_BAD_MESSAGE_FORMAT;
    
    return STATUS_OK;
}