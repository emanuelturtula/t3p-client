#include "../headers/t3p_client.h"
#include "../headers/types.h"
#include "../headers/tcp.h"
#include "../headers/menus.h"
#include <thread>

status_t t3p_client()
{
    status_t status;
    context_t context = MAIN_MENU;
    Server server;
    string playerName;
    string invitationhost;
    int connectedSockfd;
    MatchInfo matchInfo;
    ErrorHandler errorHandler;
    thread heartbeatThread(heartbeat_thread, &context, &connectedSockfd);

    while (context != CLOSE_PROGRAM)
    {
        switch(context)
        {
            case MAIN_MENU:
                main_menu(&context);
                break;
            case SEARCH_LOCAL_SERVERS_MENU:
                if ((status = search_local_servers_menu(&context, &server)) != STATUS_OK) 
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                break;
            case SEARCH_BY_IP_MENU:
                if ((status = search_by_ip_menu(&context, &server)) != STATUS_OK) 
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                break;
            case FAST_CONNECT_MENU:
                cout << "Not implemented" << endl;
                context = CLOSE_PROGRAM;
                break;
            case READY_TO_CONNECT:
                if ((status = connect_menu(&context, server)) != STATUS_OK) 
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                break;
            case CONNECT:
                get_player_name(&playerName);
                if ((status = login(server, playerName, &connectedSockfd)) != STATUS_OK) 
                {
                    cerr << "Error bad login" << endl;
                    //errorHandler.handle_error(status, &context, connectedSockfd);
                }
                else 
                    context = LOBBY_MENU;
                break;
            case LOBBY_MENU:
                if ((status = lobby_menu(&context, connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                break;
            case SEND_INVITE_MENU:
                // go to invite menu
                if ((status = invite_menu(&context, server, playerName, connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                break;
            case SEND_RANDOMINVITE_MENU:
                // go to random invite menu
                if ((status = random_invite_menu(&context, connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                break;      
            case READY_TO_PLAY:
                //Here we begin a new game. We must wait until we receive the first TURN.
                //We set up everything to begin.
                if((status = ready_to_play_context_setup(connectedSockfd, &context, &matchInfo)) != STATUS_OK)
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                break;
            case IN_A_GAME:
                if ((status = in_a_game_context(connectedSockfd, &context, matchInfo)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case LOGOUT_CONTEXT:
                if ((status = logout(&connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handle_error(status,&context,connectedSockfd);
                }
                context = MAIN_MENU;
                break;
            default:
                cout << "Context unknown. Returning to LOBBY" << endl;
                context = LOBBY_MENU;
        }   
    }

    return STATUS_OK;
}

