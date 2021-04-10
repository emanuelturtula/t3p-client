#include <iostream>
#include <string>
#include <regex>
#include <stdlib.h>
#include <list>
#include <algorithm>
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
    while ((*context) == MAIN_MENU)
    {
        cout << "MAIN MENU" << endl;
        cout << "Please select one option" << endl;
        cout << "1 - Search local servers" << endl;
        cout << "2 - Search by IP" << endl;
        getline(cin, selection);
        if (selection.compare("1") == 0)
            (*context) = SEARCH_LOCAL_SERVERS;
        else if (selection.compare("2") == 0)
            (*context) = SEARCH_BY_IP;
        else 
        {
            system("clear");
            cerr << "Error. Not an option" << endl << endl;
        }
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

    while (*context != READY_TO_CONNECT)
    {
        t3pResponseList.clear();
        serverList.clear();
        system("clear");
        cout << "SEARCH LOCAL SERVERS MENU" << endl;
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
    cout << "SEARCH BY IP MENU" << endl;
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
    cout << "CONNECT MENU" << endl;
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

status_t lobby_menu(context_t *context)
{
    /**
     * 
     */
    string selection;
    *context = LOBBY_MENU;
    while ((*context) == LOBBY_MENU)
    {
        cout << "TicTacToe LOBBY" << endl;
        cout << "Please select one option" << endl;
        cout << "1 - Invite a Player" << endl;
        cout << "2 - Random Match" << endl;
        cout << "3 - Logout" << endl;
        getline(cin, selection);
        if (selection.compare("1") == 0)
            (*context) = SEND_INVITE_MENU;
        else if (selection.compare("2") == 0)
            (*context) = SEND_RANDOMINVITE_MENU;  
        else if (selection.compare("3") == 0)
            (*context) = LOGOUT_CONTEXT;                    
        else 
        {
            system("clear");
            cerr << "Error. Not an option" << endl << endl;
        }
    }
    return STATUS_OK;
}

status_t invite_menu(context_t *context, Server server, string myPlayerName, int connectedSockfd) 
{
    /**
     * 
     */
    string selection;
    status_t status;
    T3PResponse t3pResponse;
    string availablePlayers;
    string invitePlayerName;
    vector<string> availablePlayersList;
    *context = SEND_INVITE_MENU;
    while ((*context) == SEND_INVITE_MENU)
    {
        cout << "TicTacToe INVITE" << endl;
        if ((status = send_discover(server.ip, &t3pResponse)) != STATUS_OK)
        {
            
        }
        availablePlayers = t3pResponse.dataList.front();   
        if (!parse_list_of_players(availablePlayers, &availablePlayersList))
        {
            cout << "There are no players avaiable. Press any key to go back to Lobby menu" << endl;
            (*context) = LOBBY_MENU;
            return STATUS_OK;
        }
        cout << "List of available players: " << endl;
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
            (*context) = LOBBY_MENU;
            return STATUS_OK;
        }

        cout << "Please enter the player name or \\back to go back to Lobby menu" << endl;
        getline(cin, invitePlayerName);
        
        if (find(availablePlayersList.begin(), availablePlayersList.end(), invitePlayerName) == availablePlayersList.end())
            cerr << "Error. Player is not in the list" << endl << endl; 
        else 
        {
            string message = "INVITE|" + invitePlayerName + " \r\n \r\n";
            bool response = false;
            status = invite(connectedSockfd, invitePlayerName, &response);
            switch (status)
            {
                case ERROR_BAD_PLAYER_NAME:
                    cerr << "Error. Bad player name"; 
                    break;
                case STATUS_OK:
                    
                    break;
                default: 
                    break;
            }

            // send invitation and wait for response.
        }
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