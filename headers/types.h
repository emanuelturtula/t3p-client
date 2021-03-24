#pragma once

#include <string>
#include <list>

using namespace std;

enum status_t {
    STATUS_OK,
    ERROR_SOCKET_CREATION,
    ERROR_SOCKET_CONFIGURATION,
    ERROR_SENDING_MESSAGE,
    ERROR_RECEIVING_MESSAGE,
    ERROR_NO_SERVERS_ONLINE,
    ERROR_BAD_MESSAGE_FORMAT,
    ERROR_CONNECTING
};

enum context_t {
    MAIN_MENU,
    LOBBY_MENU,
    SEARCH_BY_IP,
    SEARCH_LOCAL_SERVERS,
    READY_TO_CONNECT,
    CONNECT,
    SEND_INVITE,
    SEND_RANDOMINVITE,
    SEARCH_PLAYERS,
    LOGOUT
};

class Server {
    public:
        Server();
        Server(string new_ip);
        string ip;
        void setAvailablePlayers(string available_players);
        void setOccupiedPlayers(string occupied_players);
        list<string> playersAvailable;
        list<string> playersOccupied;
};

class T3PResponse {
    public:
        T3PResponse();
        string statusCode;
        string statusMessage;
        list<string> dataList; 
};