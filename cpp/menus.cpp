#include <iostream>
#include <string>

#include <stdlib.h>
#include "../headers/types.h"
#include "../headers/menus.h"
#include "../headers/udp.h"


using namespace std;

status_t main_menu(context_t *context)
{
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
    list<T3PResponse> t3pResponseList;
    list<Server> serverList;
    T3PResponse t3pResponse;
    status_t status;
    string selection;
    bool valid_choice = false;
    
    while (*context != READY_TO_CONNECT)
    {
        t3pResponseList.clear();
        serverList.clear();
        system("clear");
        cout << "Searching local servers" << endl;
        if ((status = send_discover_broadcast(&t3pResponseList)) != STATUS_OK)
        {
            cerr << "There are no servers available in local network." << endl;
            cout << "Do you want to scan again?" << endl;
            valid_choice = false;
            while (valid_choice == false)
            {
                cout << "Type Y to scan again or N to go back to main menu" << endl;
                getline(cin, selection);
                if (selection.compare("N") == 0)
                {
                    valid_choice = true;
                    *context = MAIN_MENU;
                    return STATUS_OK;
                }
                else if (selection.compare("Y") == 0)
                    valid_choice = true;
                else
                    cerr << "Error. Option invalid." << endl;
            }
        }
        else 
        {
            list<T3PResponse> :: iterator it1 = t3pResponseList.begin();
            for(it1; it1 != t3pResponseList.end(); ++it1)
            {
                string ip = it1->dataList.front();
                // TODO: ADD IP CHECKING
                Server tempServer(ip);
                serverList.push_back(tempServer);
            }
            
            
            cout << "Found " << serverList.size() << " servers." << endl; 
            int i = 0;
            list<Server> :: iterator it2 = serverList.begin();
            for(it2, i; it2 != serverList.end(); ++it2, ++i)
            {
                cout << i << " - " << it2->ip << endl;
            }

            valid_choice = false;
            while (valid_choice == false)
            {
                cout << "Please select a server number or type '\\back' to go back to main menu" << endl;
                getline(cin, selection);
                if (selection.compare("\\back") == 0)
                {
                    valid_choice = true;
                    *context = MAIN_MENU;
                    return STATUS_OK;
                }
                else 
                {
                    if (stoi(selection) > (serverList.size()-1))
                    {
                        cerr << "Error. That server number is not an option." << endl;
                    }
                    else
                    {
                        valid_choice = true;
                        int i = 0;
                        list<Server> :: iterator it3 = serverList.begin();
                        for(it3; i < stoi(selection); i++)
                            it3++;
                        (*server).ip = it3->ip;
                    }                                        
                }
            }
        }

        if ((status = send_discover((*server).ip, &t3pResponse)) != STATUS_OK)
        {}
        else
        {
            if (! t3pResponse.dataList.front().empty())
            {
                (*server).setAvailablePlayers(t3pResponse.dataList.front());
                t3pResponse.dataList.pop_front();
            }
            if (! t3pResponse.dataList.front().empty())
            {
                (*server).setOccupiedPlayers(t3pResponse.dataList.front());
            }
            (*context) = READY_TO_CONNECT;
        }
    }
    return STATUS_OK;
}