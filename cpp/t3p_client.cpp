#include "../headers/t3p_client.h"
#include "../headers/types.h"
#include "../headers/tcp.h"
#include "../headers/menus.h"

status_t t3p_client()
{
    status_t status;
    context_t context = MAIN_MENU;
    Server server;
    string playerName = "edturtu";
    string invitationhost;
    int connectedSockfd;
    MatchInfo matchInfo;

    while (context != CLOSE_PROGRAM)
    {
        switch(context)
        {
            case MAIN_MENU:
                if ((status = main_menu(&context)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case SEARCH_LOCAL_SERVERS:
                if ((status = search_local_servers_menu(&context, &server)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case SEARCH_BY_IP:
                if ((status = search_by_ip_menu(&context, &server)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case READY_TO_CONNECT:
                if ((status = connect_menu(&context, server)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case CONNECT:
                if ((status = login(server, playerName, &connectedSockfd)) != STATUS_OK) 
                {
                    // Handle error
                }
                else 
                    context = LOBBY_MENU;
                break;
            case LOBBY_MENU:
                if ((status = lobby_menu(&context, connectedSockfd)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case SEND_INVITE_MENU:
                // go to invite menu
                if ((status = invite_menu(&context, server, playerName, connectedSockfd)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case SEND_RANDOMINVITE_MENU:
                // go to random invite menu
                if ((status = random_invite_menu(&context, connectedSockfd)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;      
            case IN_A_GAME:
                //TODO
                break;
            case READY_TO_PLAY:
                //Here we begin a new game. We must wait until we receive the first TURN.
                //We set up everything to begin.
                if((status = ready_to_play_context_setup(connectedSockfd, &context, &matchInfo)) != STATUS_OK)
                {
                    // Handle error
                }
            case LOGOUT_CONTEXT:
                if ((status = logout(&connectedSockfd)) != STATUS_OK) 
                {
                    // Handle error
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

