#include "../headers/t3p_client.h"
#include "../headers/types.h"
#include "../headers/tcp.h"
#include "../headers/menus.h"
#include <unistd.h>
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

                }
                break;
            case SEARCH_BY_IP_MENU:
                if ((status = search_by_ip_menu(&context, &server)) != STATUS_OK) 
                {
                    
                }
                break;
            case FAST_CONNECT_MENU:
                if ((status = fast_connect_menu(&context, &server)) != STATUS_OK) 
                {
                    
                }
                break;
            case READY_TO_CONNECT:
                if ((status = connect_menu(&context, server)) != STATUS_OK) 
                {
                    
                }
                break;
            case CONNECT:
                playerName = get_player_name();
                cout << "Getting connected socket..." << endl;
                if ((status = get_connected_socket(server.ip, &connectedSockfd)) != STATUS_OK)
                {
                    cerr << "Error connecting to server" << endl;
                    sleep(2);
                    context = MAIN_MENU;
                }
                else
                {
                    cout << "OK." << endl;
                    cout << "Logging in..." << endl;
                    if ((status = login(connectedSockfd, playerName)) != STATUS_OK)
                    {
                        cerr << "Error bad login" << endl;
                        context = MAIN_MENU;
                        close(connectedSockfd);
                    }
                    else
                        context = LOBBY_MENU;
                }
                break;
            case LOBBY_MENU:
                if ((status = lobby_menu(&context, connectedSockfd)) != STATUS_OK) 
                {

                }
                break;
            case INVITE_MENU:
                if ((status = invite_menu(&context, server, playerName, connectedSockfd)) != STATUS_OK) 
                {

                }
                break;
            case RANDOMINVITE_MENU:
                if ((status = random_invite_menu(&context, connectedSockfd)) != STATUS_OK) 
                {

                }
                break;      
            case READY_TO_PLAY:
                if((status = ready_to_play_context(connectedSockfd, &context, &matchInfo)) != STATUS_OK)
                {

                }
                break;
            case IN_A_GAME:
                if ((status = in_a_game_context(connectedSockfd, &context, matchInfo)) != STATUS_OK) 
                {
                    // Handle error
                }
                break;
            case LOGOUT:
                if ((status = logout(connectedSockfd)) != STATUS_OK) 
                {
                    // Handle error
                }
                close(connectedSockfd);
                context = MAIN_MENU;
                break;
            default:
                cout << "Context unknown. Returning to LOBBY" << endl;
                context = LOBBY_MENU;
        }   
    }

    // If heartbeat thread is still running, wait until it stops
    if (heartbeatThread.joinable())
        heartbeatThread.join();

    return STATUS_OK;
}

