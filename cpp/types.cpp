#include "../headers/types.h"
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

/*Methods for MatchInformation*/

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

void MatchInfo :: parseSlots(string circleSlots, string crossSlots)
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