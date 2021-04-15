#pragma once

#include <string>
#include <iostream>
#include "types.h"
#include <poll.h>


#define TCP_PORT 2000
#define BUFFER_SIZE 1024

using namespace std;

status_t get_connected_socket(string ip, int *sockfd);
status_t login(int sockfd, string player_name);
status_t logout(int sockfd);
status_t invite(int sockfd, string player_name, bool *response);
status_t random_invite(int sockfd, T3PCommand *t3pCommand);
status_t wait_invitation_response(int sockfd, bool *accept);
status_t invitation_response(int sockfd, bool response);
status_t markslot(int sockfd, string slot);
status_t giveup(int sockfd);

void heartbeat_thread(context_t *context, int *sockfd);

status_t send_tcp_message(int sockfd, const char *message);
status_t receive_tcp_message(int sockfd, T3PResponse *t3pResponse);
status_t receive_tcp_command(int sockfd, T3PCommand *t3pCommand);
status_t parse_tcp_message(string response, T3PResponse *t3pResponse);
status_t poll_tcp_message(int sockfd, string *data_stream);
status_t parse_tcp_command(string message, T3PCommand *t3pCommand);
tcpcommand_t parse_tcp_command(string socket_message, string *argument);
status_t poll_event(int connectedSockfd, string *stdin_message, string *socket_message, int timeout=-1);
status_t peek_tcp_buffer(int sockfd, int *read_bytes, string *socket_message);