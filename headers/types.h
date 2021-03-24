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
    ERROR_CONNECTING,
    ERROR_LOGIN,
    ERROR_STATUS_MESSAGE
};

enum context_t {
    MAIN_MENU,
    SEARCH_BY_IP,
    SEARCH_LOCAL_SERVERS,
    READY_TO_CONNECT,
    CONNECT
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