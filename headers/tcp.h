#pragma once

#include <string>
#include "types.h"

// Dictionary of responses
map<status_t, string> TCPResponseDictionary = {
    {INFO_NO_PLAYERS_AVAILABLE, "101|No players available \r\n \r\n"},
    {RESPONSE_OK, "200|OK \r\n \r\n"},
    {ERROR_BAD_REQUEST, "400|Bad Request \r\n \r\n"},
    {ERROR_INCORRECT_NAME, "401|Incorrect Name \r\n \r\n"},
    {ERROR_NAME_TAKEN, "402|Name Taken \r\n \r\n"},
    {ERROR_PLAYER_NOT_FOUND, "403|Player not Found \r\n \r\n"},
    {ERROR_PLAYER_OCCUPIED, "404|Player Occupied \r\n \r\n"},
    {ERROR_BAD_SLOT, "405|Bad Slot \r\n \r\n"},
    {ERROR_NOT_TURN, "406|Not Turn"},
    {ERROR_INVALID_COMMAND, "407|Invalid command \r\n \r\n"},
    {ERROR_COMMAND_OUT_OF_CONTEXT, "408|Command out of context \r\n \r\n"},
    {ERROR_CONNECTION_LOST, "409|Connection Lost \r\n \r\n"},
    {ERROR_SERVER_ERROR, "500|Server Error \r\n \r\n"}
};

map<string, tcpcommand_t> TCPCommandTranslator = {
    {"LOGIN", LOGIN},
    {"LOGOUT", LOGOUT},
    {"INVITE", INVITE},
    {"HEARTBEAT", HEARTBEAT},
    {"ACCEPT", ACCEPT},
    {"DECLINE", DECLINE},
    {"RANDOMINVITE", RANDOMINVITE},
    {"MARKSLOT", MARKSLOT},
    {"GIVEUP", GIVEUP}
};


#define TCP_PORT 2000
#define BUFFER_SIZE 1024

using namespace std;

status_t login(Server server, string player_name, int *sockfd);
status_t logout(int *sockfd);
status_t invite(int sockfd, string player_name);
status_t random_invite(int sockfd);
status_t wait_invitation_response(int sockfd, bool *accept);
status_t invitation_response(int sockfd,bool response);

void heartbeat_thread(int sockfd);

status_t send_tcp_message(int sockfd, const char *message);
status_t receive_tcp_message(int sockfd, T3PResponse *t3pResponse);
status_t parse_tcp_message(string response, T3PResponse *t3pResponse);


