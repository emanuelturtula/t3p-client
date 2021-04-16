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
    ErrorHandler errorHandler;
    //thread heartbeatThread(heartbeat_thread, &context, &connectedSockfd);

    while (context != CLOSE_PROGRAM)
    {
        switch(context)
        {
            case MAIN_MENU:
                // This function always returns an status ok, so no error handler is needed here
                main_menu(&context);
                break;
            case SEARCH_LOCAL_SERVERS_MENU:
                /** 
                 * Search local servers can return:
                 * ERROR SOCKET CREATION
                 * ERROR SOCKET CONFIGURATION
                 * ERROR SENDING MESSAGE
                 * ERROR RECEIVING MESSAGE
                 * ERROR BAD REQUEST 
                 * Normally, when this kind of errors happen (except for BAD REQUEST), the error handler will
                 * close the socket. However, in this case, we are treating a UDP socket,
                 * which is closed before returning from the functions send_discover_broadcast
                 * or send_discover, for any status code, so we pass a NULL pointer to indicate
                 * the error handler to avoid closing the socket.
                 **/

                if ((status = search_local_servers_menu(&context, &server)) != STATUS_OK) 
                {
                    errorHandler.handleError(status, &context, NULL);
                }
                break;
            case SEARCH_BY_IP_MENU:
                /**
                 * Same as search local servers
                 * */
                if ((status = search_by_ip_menu(&context, &server)) != STATUS_OK) 
                {
                    errorHandler.handleError(status, &context, NULL);
                }
                break;
            case FAST_CONNECT_MENU:
                // This function always return status ok.
                fast_connect_menu(&context, &server);
                break;
            case READY_TO_CONNECT:
                // This function always return status ok.
                connect_menu(&context, server);
                break;
            case CONNECT:
                playerName = get_player_name();
                cout << "Getting connected socket..." << endl;
                if ((status = get_connected_socket(server.ip, &connectedSockfd)) != STATUS_OK)
                {
                    /**
                     * This function can return 
                     * ERROR SOCKET CREATION
                     * ERROR CONNECTING
                     * When treating TCP sockets, we delegate the closure to
                     * the error handler.
                     * */
                    errorHandler.handleError(status, &context, &connectedSockfd);
                }
                else
                {
                    cout << "OK." << endl;
                    cout << "Logging in..." << endl;
                    if ((status = login(connectedSockfd, playerName)) != STATUS_OK)
                    {
                        /**
                         * Login function can return
                         * BAD REQUEST (Weird that happens in this client unless we are having connection problems)
                         * INCORRECT NAME
                         * NAME TAKEN
                         * COMMAND OUT OF CONTEXT (Shouldn't happen in the client as we send the commands in a context 
                         * controlled way, meaning the user does not insert the commands manually, so commands are rarely
                         * out of context)
                         * SERVER ERROR
                         * ERROR RECEIVING MESSAGE
                         * ERROR SENDING MESSAGE
                         * 
                         * When a bad login is made, in any case (also for incorrect name or name taken), the server closes the socket,
                         * so we have to close our socket, create a new one and connect it again. However, depending on the message received,
                         * we might want to come back to main menu or to connect menu
                         * 
                         * */
                        errorHandler.handleError(status, &context, &connectedSockfd);
                    }
                    else
                        context = LOBBY_MENU;
                }
                break;
            case LOBBY_MENU:
                /**
                 * LOBBY can generate these error status:
                 * ERROR RECEIVING MESSAGE
                 * ERROR BAD REQUEST
                 * */
                if ((status = lobby_menu(&context, connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handleError(status, &context, &connectedSockfd);
                }
                break;
            case INVITE_MENU:
                /**
                 * invite menu can generate these error status:
                 * ERROR RECEIVING MESSAGE
                 * ERROR BAD REQUEST
                 * ERROR SENDING MESSAGE
                 * ERROR_BAD_PLAYER_NAME
                 * ERROR INCORRECT NAME
                 * ERROR PLAYER NOT FOUND
                 * ERROR PLAYER OCCUPIED
                 * ERROR COMMAND OUT OF CONTEXT
                 * ERROR SERVER ERROR
                 * */
                if ((status = invite_menu(&context, server, playerName, connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handleError(status, &context, &connectedSockfd);
                }
                break;
            case RANDOMINVITE_MENU:
                /**
                 * these error status can be returned:
                 * INFO NO PLAYERS AVAILABLE
                 * ERROR SENDING MESSAGE
                 * ERROR RECEIVING MESSAGE
                 * ERROR BAD REQUEST
                 * ERROR SERVER ERROR
                 * */
                if ((status = random_invite_menu(&context, connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handleError(status, &context, &connectedSockfd);
                }
                break;      
            case READY_TO_PLAY:
                /**
                 * Here we can get
                 * ERROR RECEIVING MESSAGE
                 * ERROR SETTING MATCH (this is weird, but if this happens, it is almost certain that
                 * it is because the server died or is hanged. So we want to close the socket here too)
                 * */
                if((status = ready_to_play_context(&context, connectedSockfd, &matchInfo)) != STATUS_OK)
                {
                    errorHandler.handleError(status, &context, &connectedSockfd);
                }
                break;
            case IN_A_GAME:
                /**
                 * While we are in a game, we could get
                 * ERROR RECEIVING MESSAGE
                 * ERROR SENDING MESSAGE
                 * ERROR BAD REQUEST
                 * ERROR BAD SLOT
                 * ERROR NOT TURN
                 * ERROR SERVER ERROR
                 * ERROR COMMAND OUT OF CONTEXT
                 * */
                if ((status = in_a_game_context(&context, connectedSockfd, &matchInfo)) != STATUS_OK) 
                {
                    errorHandler.handleError(status, &context, &connectedSockfd);
                }
                break;
            case LOGOUT:
                /**
                 * LOGOUT can return
                 * ERROR RECEIVING MESSAGE
                 * ERROR SENDING MESSAGE
                 * ERROR BAD REQUEST
                 * ERROR COMMAND OUT OF CONTEXT
                 * */
                if ((status = logout(connectedSockfd)) != STATUS_OK) 
                {
                    errorHandler.handleError(status, &context, &connectedSockfd);
                }
                else
                    context = MAIN_MENU;
                break;
            default:
                cout << "Context unknown. Returning to LOBBY" << endl;
                context = LOBBY_MENU;
        }   
    }

    // If heartbeat thread is still running, wait until it stops
    // if (heartbeatThread.joinable())
    //     heartbeatThread.join();

    return STATUS_OK;
}

