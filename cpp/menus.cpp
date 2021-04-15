#include <iostream>
#include <string>
#include <regex>
#include <stdlib.h>
#include <list>
#include <algorithm>
#include <unistd.h>
#include "../headers/types.h"
#include "../headers/menus.h"
#include "../headers/udp.h"
#include "../headers/tcp.h"

using namespace std;

bool scanAgain();
bool parse_list_of_players(string players, vector<string> *parsedPlayers); 

status_t main_menu(context_t *context)
{
    string selection;
    *context = MAIN_MENU;
    system("clear");
    cout << MAIN_MENU_TITLE << endl;
    while ((*context) == MAIN_MENU)
    {
        cout << "Please select one option" << endl;
        cout << "1 - Search local servers" << endl;
        cout << "2 - Search by IP" << endl;
        cout << "3 - Fast connect" << endl;
        cout << "0 - Exit program" << endl << endl;
        getline(cin, selection);
        if (selection == "1")
            (*context) = SEARCH_LOCAL_SERVERS_MENU;
        else if (selection == "2")
            (*context) = SEARCH_BY_IP_MENU;
        else if (selection == "3")
            (*context) = FAST_CONNECT_MENU;
        else if (selection == "0")
            (*context) = CLOSE_PROGRAM;
        else 
            cerr << "Error. Not an option" << endl << endl;
    }
    return STATUS_OK;
}

status_t search_local_servers_menu(context_t *context, Server *server)
{
    list<T3PResponse> t3pResponseList;
    list<Server> serverList;
    T3PResponse t3pResponse;
    status_t status;
    string selection;
    string ip;
    bool valid_choice = false;
    regex ip_checker("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    *context = SEARCH_LOCAL_SERVERS_MENU;

    system("clear");
    cout << SEARCH_LOCAL_SERVERS_MENU_TITLE << endl;
    while (*context == SEARCH_LOCAL_SERVERS_MENU)
    {
        serverList.clear();
        cout << "Searching local servers" << endl;
        if ((status = send_discover_broadcast(&t3pResponseList)) != STATUS_OK)
            return status;

        if (t3pResponseList.empty())
        {
            cout << "There are no servers available in local network." << endl;
            if (!scanAgain())
                *context = MAIN_MENU;
            return STATUS_OK;
        }
        else 
        {
            for(auto const& t3pResponseItem : t3pResponseList) 
            {
                ip = t3pResponseItem.dataList.front();
                if (regex_match(ip, ip_checker))
                {
                    Server tempServer(ip);
                    serverList.push_back(tempServer);
                }               
            }

            if (serverList.empty())
            {
                cerr << "There are servers in local network, but they are not responding correctly." << endl;
                if (!scanAgain())
                    *context = MAIN_MENU;
                return STATUS_OK;   
            }
            else
            {
                cout << "Found " << serverList.size() << " servers." << endl; 
                int i = 0;
                for(auto const& serverItem : serverList)
                {
                    cout << (i+1) << " - " << serverItem.ip << endl;
                }
                    
                valid_choice = false;
                while (valid_choice == false)
                {
                    cout << "Please select a server number or type 0 to go back to main menu" << endl;
                    getline(cin, selection);
                    if (selection == "0")
                    {
                        *context = MAIN_MENU;
                        return STATUS_OK;
                    }
                    else 
                    {
                        if (stoi(selection) > (serverList.size()))
                            cerr << "Error. That server number is not an option." << endl;
                        else
                        {
                            valid_choice = true;
                            int i = 0;
                            for(auto const& serverItem : serverList)
                            {
                                if (i == (stoi(selection)-1))
                                {
                                    (*server).ip = serverItem.ip;
                                }
                                i++;
                            }                           
                        }                                        
                    }
                }
            }
        }

        if ((status = send_discover((*server).ip, &t3pResponse)) != STATUS_OK)
        {
            cerr << "Error getting information from server" << endl;
            sleep(2);
            return status;
        }
        else
        {
            server->playersAvailable.clear();
            server->playersOccupied.clear();
            if (!t3pResponse.dataList.front().empty())
                (*server).setAvailablePlayers(t3pResponse.dataList.front());
            t3pResponse.dataList.pop_front();
            if (!t3pResponse.dataList.front().empty())
                (*server).setOccupiedPlayers(t3pResponse.dataList.front());
            (*context) = READY_TO_CONNECT;
        }
    }
    return STATUS_OK;
}

status_t search_by_ip_menu(context_t *context, Server *server)
{
    status_t status;
    bool valid_ip;
    string ip;
    regex ip_checker("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    T3PResponse t3pResponse;

    system("clear");
    cout << SEARCH_BY_IP_MENU_TITLE << endl;
    while (*context == SEARCH_BY_IP_MENU)
    {
        valid_ip = false;
        while (valid_ip == false)
        {
            cout << "Please insert the server IP or type 0 to go back to main menu" << endl;
            getline(cin, ip);
            valid_ip = regex_match(ip, ip_checker);
            if (!valid_ip)
            {
                if (ip == "0")
                {
                    *context = MAIN_MENU;
                    return STATUS_OK;
                }
                else
                    cerr << "Error. Wrong IP format." << endl;
            }
        }

        if ((status = send_discover(ip, &t3pResponse)) != STATUS_OK)
        {
            cerr << "Error getting information from server" << endl;
            sleep(2);
            return status;
        }
        else
        {
            (*server).ip = ip;
            server->playersAvailable.clear();
            server->playersOccupied.clear();
            if (!t3pResponse.dataList.front().empty())
                (*server).setAvailablePlayers(t3pResponse.dataList.front());
            t3pResponse.dataList.pop_front();
            if (!t3pResponse.dataList.front().empty())
                (*server).setOccupiedPlayers(t3pResponse.dataList.front());
            *context = READY_TO_CONNECT;
        }
    }
    return STATUS_OK;
}

status_t fast_connect_menu(context_t *context, Server *server)
{
    string ip;
    bool valid_ip;
    regex ip_checker("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

    system("clear");
    cout << FAST_CONNECT_MENU_TITLE << endl;
    *context = FAST_CONNECT_MENU;
    while(*context == FAST_CONNECT_MENU)
    {
        valid_ip = false;
        while (valid_ip == false)
        {
            cout << "Please insert the server IP or type 0 to go back to main menu" << endl;
            getline(cin, ip);
            valid_ip = regex_match(ip, ip_checker);
            if (!valid_ip)
            {
                if (ip == "0")
                {
                    *context = MAIN_MENU;
                    return STATUS_OK;
                }
                else
                    cerr << "Error. Wrong IP format." << endl;
            }
            else
            {
                server->ip = ip;
                *context = CONNECT;
            }
        }
    }

    return STATUS_OK;
}

status_t connect_menu(context_t *context, Server server)
{
    bool valid_choice;
    string choice;
    system("clear");
    cout << CONNECT_MENU_TITLE << endl;
    while (*context == READY_TO_CONNECT)
    {
        cout << "Server " << server.ip << endl;
        cout << "Players available:" << endl;;
        for (auto const& player : server.playersAvailable)
            cout << player << endl;
        cout << "Players occupied:" << endl;
        for (auto const& player : server.playersOccupied)
            cout << player << endl;
        valid_choice = false;
        while (valid_choice == false)
        {
            cout << "Type y to connect or n to go back to main menu" << endl;
            getline(cin, choice);
            if (choice == "y")
            {
                valid_choice = true;
                *context = CONNECT;
            }
            else if (choice == "n")
            {
                valid_choice = true;
                *context = MAIN_MENU;
            }
            else 
                cerr << "Error. Wrong answer." << endl;
        }
    }
    return STATUS_OK;
}

status_t lobby_menu(context_t *context, int connectedSockfd)
{
    status_t status;
    string selection;
    string socket_message;
    string invitingPlayerName;

    *context = LOBBY_MENU;
    system("clear");
    cout << LOBBY_MENU_TITLE << endl;
    while ((*context) == LOBBY_MENU)
    {
        cout << "Please select one option" << endl;
        cout << "1 - Invite a Player" << endl;
        cout << "2 - Random Match" << endl;
        cout << "3 - Logout" << endl << endl;
        poll_event(connectedSockfd, &selection, &socket_message);
        if (socket_message != "")
        {
            switch (parse_tcp_command(socket_message, &invitingPlayerName))
            {
                case INVITEFROM:
                    if ((status = received_invite_menu(context, connectedSockfd, invitingPlayerName)) != STATUS_OK)
                        return status;
                    if (*context == LOBBY_MENU)
                    {
                        system("clear");
                        cout << LOBBY_MENU_TITLE << endl;
                    }
                    break;
                default:
                    cerr << "Received a message from the server, but is not valid for this context" << endl;
                    cerr << socket_message << endl;
                    break;
            }
        }
        else 
        {
            if (selection == "1")
                (*context) = INVITE_MENU;
            else if (selection == "2")
                (*context) = RANDOMINVITE_MENU;  
            else if (selection == "3")
                (*context) = LOGOUT;                    
            else
                cerr << "Error. Not an option\n\n" << endl << endl;
        }
        
    }
    return STATUS_OK;
}

status_t invite_menu(context_t *context, Server server, string myPlayerName, int connectedSockfd) 
{
    /**
     * For this menu, we first wanted to show which players were online and available. In order to do that, 
     * we send a discover message, and then check the players available. However, we couldn't configure UDP port 
     * forwarding to be accessible through the internet. So we made a second menu were we could write directly the name of the
     * player, without depending on discover messages. We leave the first menu design in case we make it work in the future.  
     * */
    
    string selection = "";
    status_t status;
    string argument;
    T3PResponse t3pResponse;
    T3PCommand t3pCommand;
    string availablePlayers;
    string invitePlayerName;
    string socket_message;
    vector<string> availablePlayersList;
    string message;
    bool valid_input = false;

    *context = INVITE_MENU;
    system("clear");
    cout << INVITE_MENU_TITLE << endl;
    while (!valid_input)
    {
        #ifdef SHOW_PLAYERS_AVAILABLE_IN_INVITE_MENU
        if ((status = send_discover(server.ip, &t3pResponse)) != STATUS_OK)
            return status;
        else 
        {
            availablePlayers = t3pResponse.dataList.front();   
            if (!parse_list_of_players(availablePlayers, &availablePlayersList))
            {
                cout << "There are no players avaiable. Press any key to go back to Lobby menu" << endl;
                poll_event(connectedSockfd, &selection, &socket_message);
                (*context) = LOBBY_MENU;
                valid_input = true;
                return STATUS_OK;
            }
            
            vector<string> :: iterator it = find(availablePlayersList.begin(), availablePlayersList.end(), myPlayerName);
            availablePlayersList.erase(it);

            if (availablePlayersList.size() > 0)
            {
                cout << "List of available players: " << endl;
                for (auto const& player : availablePlayersList)
                    cout << player << endl; 
            }
            else
            {
                cout << "There are no players avaiable. Press any key to go back to Lobby menu" << endl;
                poll_event(connectedSockfd, &selection, &socket_message);
                (*context) = LOBBY_MENU;
                return STATUS_OK;
            }

            cout << "Please enter the player name or \\back to go back to Lobby menu" << endl;

            // Pollin stdin and socket           
            if (poll_event(connectedSockfd, &invitePlayerName, &socket_message) != STATUS_OK)
            {
                // Handle error
            }
            else 
            {
                if (socket_message != "")
                {
                    // If an invitefrom arrives, we want to decline it, as our intention is to invite another player in this context
                    switch(parse_tcp_command(socket_message, &argument))
                    {
                        case INVITEFROM:
                            send_tcp_message(connectedSockfd, "DECLINE \r\n \r\n");
                            break;
                    }
                }
                if (invitePlayerName != "")
                {
                    if (invitePlayerName == "\\back")
                    {
                        valid_input = true;
                        *context = LOBBY_MENU;
                    }
                    else if (find(availablePlayersList.begin(), availablePlayersList.end(), invitePlayerName) != availablePlayersList.end())
                    {
                        bool response = false;
                        status = invite(connectedSockfd, invitePlayerName, &response);
                        switch (status)
                        {
                            case ERROR_BAD_PLAYER_NAME:
                                cerr << "Error. Bad player name"; 
                                break;
                            case STATUS_OK:
                                if (response)
                                {
                                    cout << invitePlayerName + " accepted the invitation" << endl;
                                    (*context) = READY_TO_PLAY;
                                }
                                else
                                {
                                    cout << invitePlayerName + " declined the invitation" << endl;
                                    (*context) = LOBBY_MENU;
                                }
                                sleep(2);
                                valid_input = true;
                                break;
                            // TODO Add missing cases
                            default: 
                                break;
                        }
                    }
                    else
                        cerr << "Error. Player is not in the list" << endl << endl; 
                }
            }
        }
        #else
            cout << "Please enter the player name or type 0 to go back to lobby menu" << endl;
            if (poll_event(connectedSockfd, &invitePlayerName, &socket_message) != STATUS_OK)
                return status;
            if (socket_message != "")
            {
                switch(parse_tcp_command(socket_message, &argument))
                {
                    case INVITEFROM:
                        if ((status = send_tcp_message(connectedSockfd, "DECLINE \r\n \r\n")) != STATUS_OK)
                            return status;
                        break;
                    default:
                        break;
                }
            }
            if (invitePlayerName != "")
            {
                if (isPlayerNameCorrect(invitePlayerName))
                {
                    message = "INVITE|" + invitePlayerName + " \r\n \r\n";
                    if ((status = invite(connectedSockfd, invitePlayerName, &t3pCommand)) != STATUS_OK)
                        return status;
                    if (t3pCommand.command == "ACCEPT")
                    {
                        cout << invitePlayerName + " accepted!" << endl;
                        *context = READY_TO_PLAY;
                        valid_input = true;
                        sleep(1);
                    }
                    else if (t3pCommand.command == "DECLINE")
                    {
                        cout << invitePlayerName + " declined :(" << endl;
                        valid_input = true; 
                        sleep(1);
                    }
                    else if (t3pCommand.command == "INVITEFROM")
                    {
                        // This is weird, we shouldn't receive an INVITEFROM once we send an INVITE, because
                        // in the server we changed our context and the server knows we are 'occupied'. So
                        // the server will not allow another client to send an invitation to our name. However,
                        // just in case, we consider the possibility to receive this and decline it.
                        if ((status = send_tcp_message(connectedSockfd, "DECLINE \r\n \r\n")) != STATUS_OK)
                            return status;
                    }                
                }
                else
                {
                    if (invitePlayerName == "0")
                    {
                        *context = LOBBY_MENU;
                        valid_input = true;
                    }
                    else
                    {
                        cout << "This name is not allowed, please select another" << endl;
                    }
                }
            }
        #endif
    }
    return STATUS_OK;
}

status_t received_invite_menu(context_t *context, int connectedSockfd, string invitingPlayerName)
{
    status_t status;
    string selection = "";
    string argument;
    string socket_message = "";

    system("clear");
    cout << RECEIVED_INVITE_MENU << endl;
    cout << invitingPlayerName + " is inviting you to a match. Type y to accept or n to decline.\n\n";
    bool waiting_answer = true;
    while (waiting_answer)
    {
        if ((status = poll_event(connectedSockfd, &selection, &socket_message)) != STATUS_OK)
            return status;
        if (socket_message != "")
        {
            switch(parse_tcp_command(socket_message, &argument))
            {
                case INVITATIONTIMEOUT:
                    cout << "The invitation will be declined due to timeout" << endl;
                    sleep(2);
                    waiting_answer = false;
                    break;
            }
        }
        else
        {
            if (selection == "y")
            {
                send_tcp_message(connectedSockfd, "ACCEPT \r\n \r\n");
                waiting_answer = false;
                *context = READY_TO_PLAY;
            }
            else if (selection == "n")
            {
                send_tcp_message(connectedSockfd, "DECLINE \r\n \r\n");
                waiting_answer = false;
                *context = LOBBY_MENU;
            }
            else
                cerr << "Error. Not an option." << endl;
        }
    }
    return STATUS_OK;
}

status_t random_invite_menu(context_t *context, int connectedSockfd)
{
    status_t status;
    T3PCommand t3pCommand;
    *context = RANDOMINVITE_MENU;
    system("clear");

    cout << RANDOMINVITE_MENU_TITLE << endl;
    cout << "Sending random invite message..." << endl;
    if ((status = random_invite(connectedSockfd, &t3pCommand)) != STATUS_OK)
        return status;
    
    if (t3pCommand.command == "ACCEPT")
    {
        cout << "A player accepted!" << endl;
        (*context) = READY_TO_PLAY;
    }
    else if (t3pCommand.command == "DECLINE") 
    {
        cout << "The invited player declined the invitation :(. Going back to lobby" << endl;
        (*context) = LOBBY_MENU;
        sleep(2);
    }
    else 
    {
        cerr << "Error. This command is unknown. Going back to lobby" << endl;
        (*context) = LOBBY_MENU;
        sleep(2);
    }

    return STATUS_OK;
}

status_t ready_to_play_context(context_t *context, int sockfd, MatchInfo *matchInfo)
{
    status_t status;
    string first_turn;
    T3PCommand t3pCommand;

    *context = READY_TO_PLAY;

    matchInfo->clearSlots(); 
    matchInfo->playerSymbol = EMPTY;

    cout << "Waiting server information to start the match" << endl;

    // Receive the command TURNPLAY or TURNWAIT
    if ((status = receive_tcp_command(sockfd, &t3pCommand)) != STATUS_OK)
        return status;

    if (t3pCommand.command == "TURNPLAY")
    {
        // CIRCLE player is always the one to start
        matchInfo->playerSymbol = CIRCLE;
        matchInfo->myTurn = true;
        *context = IN_A_GAME;
    }
    else if (t3pCommand.command == "TURNWAIT")
    {
        matchInfo->playerSymbol = CROSS;
        matchInfo->myTurn = false;
        *context = IN_A_GAME;
    }
    else 
    {
        //Weird case
        return ERROR_SETTING_MATCH;
    }
        
    return STATUS_OK;
}

status_t in_a_game_context(int sockfd, context_t *context, MatchInfo matchInfo)
{
    status_t status;
    string stdin_message;
    string socket_message;
    string message;
    int slot_number;
    T3PCommand t3pCommand;
    string circleSlots;
    string crossSlots;
    string slots;
    bool valid_input = false;
    *context = IN_A_GAME;
    system("clear");
    if (matchInfo.myTurn)
    {
        matchInfo.playAsCirle = true;
        cout << "You play as CIRCLE\n\n\n" << endl;
    }
    else
    {
        matchInfo.playAsCirle = false;
        cout << "You play as CROSS\n\n\n" << endl;
    }
        
    while (*context == IN_A_GAME)
    {
        // print the board
        matchInfo.printSlots();
        if (matchInfo.myTurn)
        {
            cout << "It's your turn. Please choose the next slot number to mark." << endl;
            matchInfo.printSlotsNumpad();
            cout << "To giveup match, please write 0.\n" << endl;
            // If it is my turn, I need to tell the player to enter a slot number
            valid_input = false;
            while (valid_input == false)
            {
                if ((status = poll_event(sockfd, &stdin_message, &socket_message)) != STATUS_OK)
                {
                    // Handle error
                }  
                if (socket_message != "")
                {
                    // If we got here, probably it's because we lost due to timeout. Also it could happen
                    // that the other player got disconnected so it's a connection lost message.
                    if (socket_message.find("MATCHEND|TIMEOUT") != string::npos)
                        cout << "You lost due to timeout" << endl;
                    else if (socket_message.find("MATCHEND|CONNECTIONLOST") != string::npos)
                        cout << "The other player lost connection" << endl;
                    *context = LOBBY_MENU;
                    sleep(2);
                    return STATUS_OK;
                }
                else if (stdin_message != "")
                {
                    // If the entered message is different from a number from 0 to 9, tell the user that
                    // it is wrong.
                    if ((stdin_message.size() > 1) || (stdin_message.find_first_not_of("0123456789") != string::npos))
                        cerr << "Error. Bad option. Please enter another one";
                    else
                    {
                        if (stdin_message == "0")
                        {
                            // 0 is to giveup.
                            if ((status = giveup(sockfd)) != STATUS_OK)
                            {
                                // handle error
                            }
                            *context = LOBBY_MENU;
                            valid_input = true;
                        }
                        else 
                        {
                            slot_number = stoi(stdin_message)-1;
                            if (matchInfo.getSlots()[slot_number] != EMPTY)
                                cerr << "Error. The slot is occupied. Please enter another one" << endl;
                            else 
                            {
                                if ((status = markslot(sockfd, stdin_message)) != STATUS_OK)
                                {
                                    // handle error
                                }
                                valid_input = true;
                                if (matchInfo.playAsCirle)
                                    matchInfo.setSlots(stdin_message, "");
                                else 
                                    matchInfo.setSlots("", stdin_message);
                                matchInfo.printSlots();
                            }
                        }
                    }
                }
            }
        }
        else
        { 
            // if it is not my turn
            system("clear");
            matchInfo.printSlots();
            cout << "Waiting other player's move." << endl;
        }

        // Wait till the Server tells us TURNWAIT, TURNPLAY or MATCHEND.
        // TURNWAIT and TURNPLAY needs to be answered with a 200 OK.
        if ((status = receive_tcp_command(sockfd, &t3pCommand)) != STATUS_OK)
        {
            cerr << "Error receiving command" << endl;
            return STATUS_OK;
        }
            
        cout << t3pCommand.command << endl;

        if (t3pCommand.command == "TURNWAIT")
        {
            if ((status = send_tcp_message(sockfd, "200|OK \r\n \r\n")) != STATUS_OK)
            {
                // handle error
            }
            matchInfo.myTurn = false;
            slots = t3pCommand.dataList.front();
            crossSlots = slots.substr(0, slots.find("|"));
            slots.erase(0, slots.find("|") + 1);
            circleSlots = slots.substr(0, slots.find(" \r\n \r\n"));
            matchInfo.setSlots(circleSlots, crossSlots);
        }
        else if (t3pCommand.command == "TURNPLAY")
        {
            if ((status = send_tcp_message(sockfd, "200|OK \r\n \r\n")) != STATUS_OK)
            {
                // handle error
            }
            matchInfo.myTurn = true;
            slots = t3pCommand.dataList.front();
            crossSlots = slots.substr(0, slots.find("|"));
            slots.erase(0, slots.find("|") + 1);
            circleSlots = slots.substr(0, slots.find(" \r\n \r\n"));
            matchInfo.setSlots(circleSlots, crossSlots);
        }
        else if (t3pCommand.command == "MATCHEND")
        {
            if (t3pCommand.dataList.front().find("YOUWIN") != string::npos)
                cout << "YOU WON!!" << endl;
            else if (t3pCommand.dataList.front().find("TIMEOUTWIN") != string::npos)
                cout << "You win due to timeout!" << endl;
            else if (t3pCommand.dataList.front().find("YOULOSE") != string::npos)
                cout << "YOU LOST!!" << endl;
            else if (t3pCommand.dataList.front().find("TIMEOUTLOSE") != string::npos)
                cout << "You lost due to timeout!" << endl;
            else if (t3pCommand.dataList.front().find("DRAW") != string::npos)
                cout << "Match ended in draw!" << endl;
            else if (t3pCommand.dataList.front().find("CONNECTIONLOST") != string::npos)
                cout << "The other player lost connection" << endl;
            

            sleep(5);
            *context = LOBBY_MENU;
        }
    }
    return STATUS_OK;
}

string get_player_name()
{
    string playerName;
    bool nameCorrect = false;
    while(nameCorrect == false)
    {
        cout << "Please, enter your player name:" << endl;
        cout << "(Min. 3 and Max. 20 characters long. Only alphabetic characters allowed)" << endl;
        getline(cin, playerName);
        nameCorrect = isPlayerNameCorrect(playerName);
        if (!nameCorrect)
            cout << "This name is not allowed, please select another" << endl;
    }
    return playerName;
}

bool isPlayerNameCorrect(string playerName)
{
    bool nameCorrect = false;
    regex playerNameChecker("^[a-zA-Z]+$");
    if ((playerName.size() < 3 || playerName.size() > 20) ||
        (!regex_match(playerName, playerNameChecker)));
    else
        nameCorrect = true;
    return nameCorrect;
}

bool scanAgain()
{
    string selection;
    cout << "Do you want to scan again?" << endl;
    while (1)
    {
        cout << "Type Y to scan again or N to go back to main menu" << endl;
        getline(cin, selection);
        if (selection.compare("N") == 0)
            return false;
        else if (selection.compare("Y") == 0)
            return true;
        else
            cerr << "Error. Option invalid." << endl;
    }
}

bool parse_list_of_players(string players, vector<string> *parsedPlayers)
{
    size_t pos;
    (*parsedPlayers).clear();
    if (players != "")
    {
        while ((pos = players.find("|")) != string::npos)
        {
            (*parsedPlayers).push_back(players.substr(0, pos));
            players.erase(0, pos+1);
        }
        (*parsedPlayers).push_back(players.substr(0));
        return true;  
    }

    return false; 
}