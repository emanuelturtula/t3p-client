#pragma once

#include "../headers/types.h"
#include <string.h>
#include <chrono>
#include "../headers/tcp.h"

status_t main_menu(context_t *context);
status_t search_local_servers_menu(context_t *context, Server *server);
status_t search_by_ip_menu(context_t *context, Server *server);
status_t connect_menu(context_t *context, Server server);
status_t lobby_menu(int sockfd,context_t *context, string *invitationhost);
status_t invitation_from_menu(int sockfd, context_t *context, string invitationHost);
status_t ready_to_play_context_setup(int sockfd, context_t *context, MatchInfo *matchInfo);