#include "../headers/types.h"
#include <string>
#include <list>

using namespace std;

list<string> parse_players_list(string players);

T3PResponse :: T3PResponse()
{

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
