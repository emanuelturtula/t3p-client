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
    ERROR_BAD_MESSAGE_FORMAT
};

class Server {
    public:
        Server();
        string ip;
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