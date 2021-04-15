#include "../headers/types.h"
#include "../headers/tcp.h"
#include <unistd.h>
#include <string>
#include <list>

using namespace std;

list<string> parse_players_list(string players);

T3PResponse :: T3PResponse()
{

}

T3PCommand :: T3PCommand()
{
    this->command = "";
    this->dataList.clear();
}

void T3PCommand :: clear()
{
    this->command = "";
    this->isNewCommand = false;
    this->dataList.clear();
}

Server :: Server()
{
    
}

Server :: Server(string new_ip)
{
    ip = new_ip;
}

void Server :: setAvailablePlayers(string available_players)
{
    playersAvailable = parse_players_list(available_players);
}

void Server :: setOccupiedPlayers(string occupied_players)
{
    playersOccupied = parse_players_list(occupied_players);
}

list<string> parse_players_list(string players)
{
    size_t pos;
    char delimiter = '|';
    list<string> playersList;
    
    while((pos = players.find(delimiter)) != string :: npos)
    {
        playersList.push_back(players.substr(0, pos));
        players.erase(0, pos+1);
    }

    // Last item does not have the delimiter, so we ensure we add it.
    playersList.push_back(players.substr(0));
    return playersList;
}

/**
 * Methods for Slot
 * */
Slot :: Slot()
{
    this->available = true; 
}

void Slot :: clear()
{
    this->available = true;
}

/**
 * Methods for MatchInfo
 * */

MatchInfo :: MatchInfo()
{
    int i;
    this->playerSymbol = EMPTY;
    this->myTurn = false;
    this->slots.resize(9);
    this->clearSlots();
}

void MatchInfo :: clearSlots()
{
    int i;
    for (i = 0; i < this->slots.size(); i++)
    {
        this->slots[i] = EMPTY;
    }
}

void MatchInfo::printSlots(){

    size_t line = 1;
    vector<MatchSlot> printVector = this->slots;

    cout << "Current state of Game:" << endl << "===========" << endl;

    for (size_t line = 3; line > 0; line--){
        for (size_t i = 0+((line-1)*3); i < line*3; i++){
    
            if(printVector[i] == CIRCLE)
                cout << " o ";
            
            else if(printVector[i] == CROSS)
                cout << " x ";
    
            else 
                cout << "   ";
    
            if (i != 2 && i != 5 && i !=8)
                cout << "|";
            else
                cout << endl;
        }
        if (line !=1)
            cout << "-----------" << endl;
    }
    
    cout << "===========" << endl << endl;
}

void MatchInfo :: setSlots(string circleSlots, string crossSlots)
{
    int idx;
    if (circleSlots != "")
    {
        for (char& c : circleSlots)
        {
            idx = (int)c - 49;
            this->slots[idx] = CIRCLE;
        }
    }
    if (crossSlots != "")
    {
        for (char& c : crossSlots)
        {
            idx = (int)c - 49;
            this->slots[idx] = CROSS;
        }
    }
}

vector<MatchSlot> MatchInfo :: getSlots()
{
    return this->slots;
}

void MatchInfo :: printSlotsNumpad()
{
    
    cout << "Please choose the next slot number to mark." << endl;
    
    cout << " 7 | 8 | 9 " << endl;
    cout << "-----------" << endl;
    cout << " 4 | 5 | 6 " << endl;
    cout << "-----------" << endl;
    cout << " 1 | 2 | 3 " << endl;
}

map<string, status_t> T3PStatusCodeMapper {
    {"101", INFO_NO_PLAYERS_AVAILABLE},
    {"200", STATUS_OK},
    {"400", ERROR_BAD_REQUEST},
    {"401", ERROR_INCORRECT_NAME},
    {"402", ERROR_NAME_TAKEN},
    {"403", ERROR_PLAYER_NOT_FOUND},
    {"404", ERROR_PLAYER_OCCUPIED},
    {"405", ERROR_BAD_SLOT},
    {"406", ERROR_NOT_TURN},
    {"407", ERROR_INVALID_COMMAND},
    {"408", ERROR_COMMAND_OUT_OF_CONTEXT},
    {"500", ERROR_SERVER_ERROR}
};

map<string, tcpcommand_t> TCPCommandTranslator = {
    {"INVITEFROM", INVITEFROM},
    {"INVITATIONTIMEOUT", INVITATIONTIMEOUT},
    {"ACCEPT", ACCEPT},
    {"DECLINE", DECLINE},
    {"TURNPLAY", TURNPLAY},
    {"TURNWAIT", TURNWAIT},
    {"MATCHEND", MATCHEND}
};

map<status_t, string> StatusTranslator = {
    {INFO_NO_PLAYERS_AVAILABLE, "INFORMATION - No players available"},
    {ERROR_BAD_REQUEST, "ERROR - Bad request"},
    {ERROR_INCORRECT_NAME, "ERROR - Incorrect name"},
    {ERROR_NAME_TAKEN, "ERROR - Name taken"},
    {ERROR_PLAYER_NOT_FOUND, "ERROR - Player not found"},
    {ERROR_PLAYER_OCCUPIED, "ERROR - Player occupied"},
    {ERROR_BAD_SLOT, "ERROR - Bad slot"},
    {ERROR_NOT_TURN, "ERROR - Not your turn"},
    {ERROR_INVALID_COMMAND, "ERROR - Invalid command"},
    {ERROR_COMMAND_OUT_OF_CONTEXT, "ERROR - Command out of context"},
    {ERROR_SERVER_ERROR, "ERROR - Server error"},
    {ERROR_RECEIVING_MESSAGE, "ERROR - Receiving message"},
    {ERROR_SENDING_MESSAGE, "ERROR - Sending message"},
    {ERROR_SOCKET_CREATION, "ERROR - Socket creation"},
    {ERROR_NO_SERVERS_ONLINE, "ERROR - No servers online"},
    {ERROR_SETTING_MATCH, "ERROR - Setting match failed"},
    {ERROR_CONNECTING, "ERROR - Connecting"}
};


ErrorHandler :: ErrorHandler()
{

}

void ErrorHandler :: printError(status_t status)
{
    cerr << StatusTranslator[status] << endl;
}

void ErrorHandler :: handleError(status_t status, context_t *context, int *socket)
{
    this->printError(status);
    if ((status == ERROR_SENDING_MESSAGE) ||
        (status == ERROR_RECEIVING_MESSAGE) ||
        (status == ERROR_SERVER_ERROR) ||
        (status == ERROR_SETTING_MATCH))
    {
        if (socket != NULL)
            close(*socket);
        *context = MAIN_MENU;
    }
    else
    {
        switch(status)
        {
            case INFO_NO_PLAYERS_AVAILABLE:
                *context = LOBBY_MENU;
                break;
            default:
                break;
        }
    }
}


