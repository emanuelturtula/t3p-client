
#include <string>
#include <mutex>
#include "Server.h"

using namespace std;

mutex m;


// Constructor

Server :: Server(string ip)
{
    ip = ip;
}


// Getters

string Server :: getIp()
{
    lock_guard<mutex> guard(m);
    return ip;
}

string Server :: getName()
{
    lock_guard<mutex> guard(m);
    return name;
}

size_t Server :: getPlayersOnline()
{
    lock_guard<mutex> guard(m);
    return players_online;
}

size_t Server :: getPlayersOccupied()
{
    lock_guard<mutex> guard(m);
    return players_occupied;
}

bool Server :: isSelected()
{
    lock_guard<mutex> guard(m);
    return selected;
}


// Setters 

void Server :: setIp(string ip)
{
    lock_guard<mutex> guard(m);
    ip = ip;
}   

void Server :: setName(string name)
{
    lock_guard<mutex> guard(m);
    name = name;
}  

void Server :: setPlayersOnline(size_t players)
{
    lock_guard<mutex> guard(m);
    players_online = players;
}   

void Server :: setPlayersOccupied(size_t players)
{
    lock_guard<mutex> guard(m);
    players_occupied = players;
}   

void Server :: setSelected(bool selection)
{
    lock_guard<mutex> guard(m);
    selected = selection;
}