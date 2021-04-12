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
    /**
     * 
     */
    string selection;
    *context = MAIN_MENU;
    system("clear");
    cout << MAIN_MENU_TITLE << endl;
    while ((*context) == MAIN_MENU)
    {
        cout << "Please select one option" << endl;
        cout << "1 - Search local servers" << endl;
        cout << "2 - Search by IP" << endl << endl;
        getline(cin, selection);
        if (selection.compare("1") == 0)
            (*context) = SEARCH_LOCAL_SERVERS;
        else if (selection.compare("2") == 0)
            (*context) = SEARCH_BY_IP;
        else 
            cerr << "Error. Not an option" << endl << endl;
    }
    return STATUS_OK;
}

status_t search_local_servers_menu(context_t *context, Server *server)
{
    /**
     * 
     */
    list<T3PResponse> t3pResponseList;
    list<Server> serverList;
    T3PResponse t3pResponse;
    status_t status;
    string selection;
    bool valid_choice = false;
    regex ip_checker("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    system("clear");
    cout << SEARCH_LOCAL_SERVERS_MENU_TITLE << endl;
    while (*context != READY_TO_CONNECT)
    {
        t3pResponseList.clear();
        serverList.clear();
        cout << "Searching local servers" << endl;
        if ((status = send_discover_broadcast(&t3pResponseList)) != STATUS_OK)
        {
            cerr << "There are no servers available in local network." << endl;
            if (!scanAgain())
            {
                *context = MAIN_MENU;
                return STATUS_OK;
            }    
        }
        else 
        {
            for(auto const& t3pResponseItem : t3pResponseList) 
            {
                string ip = t3pResponseItem.dataList.front();
                if (regex_match(ip, ip_checker))
                {
                    Server tempServer(ip);
                    serverList.push_back(tempServer);
                }               
            }

            if (serverList.empty())
            {
                cerr << "There servers in local network, but they are not responding correctly." << endl;
                if (!scanAgain())
                {
                    *context = MAIN_MENU;
                    return STATUS_OK;
                }    
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
                    cout << "Please select a server number or type '\\back' to go back to main menu" << endl;
                    getline(cin, selection);
                    if (selection.compare("\\back") == 0)
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
            cerr << "Error getting information from server" << endl;
        else
        {
            if (!t3pResponse.dataList.front().empty())
            {
                (*server).setAvailablePlayers(t3pResponse.dataList.front());
            }
            t3pResponse.dataList.pop_front();
            if (!t3pResponse.dataList.front().empty())
            {
                (*server).setOccupiedPlayers(t3pResponse.dataList.front());
            }
            (*context) = READY_TO_CONNECT;
        }
    }
    return STATUS_OK;
}

status_t search_by_ip_menu(context_t *context, Server *server)
{
    status_t status;
    bool valid_ip;
    regex ip_checker("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    T3PResponse t3pResponse;

    system("clear");
    cout << SEARCH_BY_IP_MENU_TITLE << endl;
    while (*context == SEARCH_BY_IP)
    {
        string ip;
        valid_ip = false;
        while (valid_ip == false)
        {
            cout << "Please insert the server IP or type \\back to go back to main menu" << endl;
            getline(cin, ip);
            valid_ip = regex_match(ip, ip_checker);
            if (!valid_ip)
            {
                if (ip.compare("\\back") == 0)
                {
                    *context = MAIN_MENU;
                    return STATUS_OK;
                }
                else
                    cerr << "Error. Wrong IP format." << endl;
            }
        }

        if ((status = send_discover(ip, &t3pResponse)) != STATUS_OK)
            cerr << "Error getting information from server" << endl;
        else
        {
            (*server).ip = ip;
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
            cout << "Type Y to connect or N to go back to main menu" << endl;
            getline(cin, choice);
            if (choice.compare("Y") == 0)
            {
                valid_choice = true;
                *context = CONNECT;
            }
            else if (choice.compare("N") == 0)
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
    /**
     * 
     */
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
                    {
                        // Handle error
                    }
                    if (*context == LOBBY_MENU)
                    {
                        system("clear");
                        cout << LOBBY_MENU_TITLE << endl;
                    }
                    break;
            }
        }
        else 
        {
            if (selection.compare("1") == 0)
                (*context) = SEND_INVITE_MENU;
            else if (selection.compare("2") == 0)
                (*context) = SEND_RANDOMINVITE_MENU;  
            else if (selection.compare("3") == 0)
                (*context) = LOGOUT_CONTEXT;                    
            else 
                cerr << "Error. Not an option\n\n" << endl << endl;
        }
        
    }
    return STATUS_OK;
}

status_t invite_menu(context_t *context, Server server, string myPlayerName, int connectedSockfd) 
{
    /**
     * 
     */
    string selection = "";
    status_t status;
    string argument;
    T3PResponse t3pResponse;
    string availablePlayers;
    string invitePlayerName;
    string socket_message;
    vector<string> availablePlayersList;
    bool valid_input = false;

    *context = SEND_INVITE_MENU;
    system("clear");
    cout << INVITE_MENU_TITLE << endl;
    while (!valid_input)
    {
        if ((status = send_discover(server.ip, &t3pResponse)) != STATUS_OK)
        {
            //Handle error
        }
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
    }
    return STATUS_OK;
}

status_t received_invite_menu(context_t *context, int connectedSockfd, string invitingPlayerName)
{
    string selection = "";
    string argument;
    string socket_message = "";
    system("clear");
    cout << RECEIVED_INVITE_MENU << endl;
    cout << invitingPlayerName + " is inviting you to a match. Type Y to accept or N to decline.\n\n";
    bool waiting_answer = true;
    while (waiting_answer)
    {
        poll_event(connectedSockfd, &selection, &socket_message);
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
            if (selection == "Y")
            {
                send_tcp_message(connectedSockfd, "ACCEPT \r\n \r\n");
                waiting_answer = false;
                *context = READY_TO_PLAY;
            }
            else if (selection == "N")
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
    *context = SEND_RANDOMINVITE_MENU;
    system("clear");

    cout << RANDOMINVITE_MENU_TITLE << endl;
    cout << "Sending random invite message..." << endl;
    if ((status = random_invite(connectedSockfd, &t3pCommand)) != STATUS_OK)
    {
        if (status == INFO_NO_PLAYERS_AVAILABLE)
            cout << "No players available" << endl;
        else
            cerr << "Error. Random invite failed" << endl;
        *context = LOBBY_MENU;
        sleep(2);
        return status;
    }
    
    if (t3pCommand.command == "ACCEPT")
    {
        cout << "A player accepted!" << endl;
        (*context) = READY_TO_PLAY;
        sleep(2);
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

status_t ready_to_play_context_setup(int sockfd, context_t *context, MatchInfo *matchInfo){

    status_t status;
    string first_turn;
    T3PServerMessages t3pserverMessage;
    matchInfo->clearSlots(); // we set a empty MatchInfo
    matchInfo->playerSymbol = EMPTY;

    cout << "Beginning new MATCH" << endl;

    // Here we should get a first TURN:
    if ( (status = poll_tcp_message(sockfd, &first_turn)) != STATUS_OK)
        return status;

        
    if( (status = t3pserverMessage.parse_buffer(first_turn)) != STATUS_OK)
    {
        *context = LOBBY_MENU;
        cerr << "Error. Server didn't send match start" << endl;
        sleep(2);
        return status;
    }


    if (first_turn == ""){
        return ERROR_READY_TO_PLAY_MATCH_NOT_SET_FROM_SERVER;
    }

    // Here we check wich turn is it and we set up "matchInfo"
    if(t3pserverMessage.getName() == "TURNPLAY"){ // We are circle (circle starts)
        matchInfo->playerSymbol = CIRCLE;
        matchInfo->myTurn = true;
        *context = IN_A_GAME;

    } else if (t3pserverMessage.getName() == "TURNWAIT"){ // We are cross
        matchInfo->playerSymbol = CROSS;
        matchInfo->myTurn = false;
        *context = IN_A_GAME;
    }
    else{
        (*context) = LOBBY_MENU;
        return ERROR_READY_TO_PLAY_MATCH_NOT_SET_FROM_SERVER;
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