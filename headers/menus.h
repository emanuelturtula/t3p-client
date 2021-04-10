#pragma once

#include "../headers/types.h"

status_t main_menu(context_t *context);
status_t search_local_servers_menu(context_t *context, Server *server);
status_t search_by_ip_menu(context_t *context, Server *server);
status_t connect_menu(context_t *context, Server server);
status_t lobby_menu(context_t *context);
status_t invite_menu(context_t *context, Server server, string myPlayerName, int connectedSockfd);