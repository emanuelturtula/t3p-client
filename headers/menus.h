#pragma once

#include "../headers/types.h"

status_t main_menu(context_t *context);
status_t search_local_servers_menu(context_t *context, Server *server);
status_t search_by_ip_menu(context_t *context, Server *server);
status_t connect_menu(context_t *context, Server server);
status_t lobby_menu(context_t *context, int connectedSockfd);
status_t invite_menu(context_t *context, Server server, string myPlayerName, int connectedSockfd);
status_t received_invite_menu(context_t *context, int connectedSockfd, string invitingPlayerName);

#define LOBBY_MENU_TITLE "TicTacToe LOBBY\n----------------------------------\n\n"
#define INVITE_MENU_TITLE "TicTacToe INVITE\n----------------------------------\n\n"
#define MAIN_MENU_TITLE "TicTacToe MAIN MENU\n----------------------------------\n\n"
#define SEARCH_LOCAL_SERVERS_MENU_TITLE "TicTacToe SEARCH LOCAL SERVERS\n----------------------------------\n\n"
#define SEARCH_BY_IP_MENU_TITLE "TicTacToe SEARCH BY IP\n----------------------------------\n\n"
#define CONNECT_MENU_TITLE "TicTacToe CONNECT MENU\n----------------------------------\n\n"
#define RECEIVED_INVITE_MENU "You received an invitation!\n----------------------------------\n\n"