#pragma once

#include <string>
#include "types.h"
#include <poll.h>


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

status_t poll_tcp_message_or_stdin(int sockfd, context_t *context);


