#include <iostream>
#include <string>

#include <stdlib.h>
#include "../headers/types.h"
#include "../headers/menus.h"


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