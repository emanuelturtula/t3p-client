#include <iostream>
#include <string>
#include <regex>
#include <stdlib.h>
#include "../headers/types.h"
#include "../headers/menus.h"
#include "../headers/udp.h"


bool scanAgain();

using namespace std;

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