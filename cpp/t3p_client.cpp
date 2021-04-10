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
    while (context != LOGOUT_CONTEXT)
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
                if ((status = lobby_menu(connectedSockfd,&context, &invitationhost)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;

            case INVITATIONFROM:
               if ((status = invitation_from_menu(connectedSockfd,&context, invitationhost)) != STATUS_OK)
               {

                   // Handle error
               }

            case SEND_INVITE:
                
                // go to invite menu
                break;
            case SEND_RANDOMINVITE:
                // go to random invite menu
                break;      
            case SEARCH_PLAYERS:
                // go to search players menu
                break;
            case IN_A_GAME:
                //TODO
                break;
            default:
                context = LOBBY_MENU;

            
        }   
    }

    return STATUS_OK;
}

