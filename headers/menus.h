#pragma once

#include "../headers/types.h"
#include <string.h>
#include <chrono>
#include "../headers/tcp.h"

status_t main_menu(context_t *context);
status_t search_local_servers_menu(context_t *context, Server *server);
status_t search_by_ip_menu(context_t *context, Server *server);
status_t connect_menu(context_t *context, Server server);

//status_t lobby_menu(int sockfd,context_t *context, string *invitationhost);
status_t invitation_from_menu(int sockfd, context_t *context, string invitationHost);
status_t ready_to_play_context_setup(int sockfd, context_t *context, MatchInfo *matchInfo);

status_t lobby_menu(context_t *context, int connectedSockfd);
status_t invite_menu(context_t *context, Server server, string myPlayerName, int connectedSockfd);
status_t received_invite_menu(context_t *context, int connectedSockfd, string invitingPlayerName);
status_t random_invite_menu(context_t *context, int connectedSockfd);

#define LOBBY_MENU_TITLE "TicTacToe LOBBY\n----------------------------------\n\n"
#define INVITE_MENU_TITLE "TicTacToe INVITE\n----------------------------------\n\n"
#define MAIN_MENU_TITLE "TicTacToe MAIN MENU\n----------------------------------\n\n"
#define SEARCH_LOCAL_SERVERS_MENU_TITLE "TicTacToe SEARCH LOCAL SERVERS\n----------------------------------\n\n"
#define SEARCH_BY_IP_MENU_TITLE "TicTacToe SEARCH BY IP\n----------------------------------\n\n"
#define CONNECT_MENU_TITLE "TicTacToe CONNECT MENU\n----------------------------------\n\n"
#define RECEIVED_INVITE_MENU "You received an invitation!\n----------------------------------\n\n"
#define RANDOMINVITE_MENU_TITLE "TicTacToe RANDOMINVITE\n----------------------------------\n\n"

