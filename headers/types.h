#pragma once

#include <string>
#include <list>
#include <map>

using namespace std;

enum status_t {
    STATUS_OK= 0,
    ERROR_SOCKET_CREATION,
    ERROR_SOCKET_CONFIGURATION,
    ERROR_SOCKET_LISTENING,
    ERROR_SOCKET_READING,
    ERROR_SOCKET_BINDING,
    ERROR_SENDING_MESSAGE,
    ERROR_RECEIVING_MESSAGE,
    ERROR_NO_SERVERS_ONLINE,
    ERROR_BAD_MESSAGE_FORMAT,
    ERROR_CONNECTING,
    ERROR_LOGIN,
    ERROR_STATUS_MESSAGE,
    ERROR_BAD_PLAYER_NAME,
    ERROR_OUT_OF_CONTEXT,
    ERROR_UNEXPECTED_EVENT_POLL_TCP_INVITATION_FROM,
    ERROR_NULL_POINTER,

    /*******************************/
    // RFC Responses:
    //1xx: Informative responses
    INFO_NO_PLAYERS_AVAILABLE = 100,
    //2xx: Correct petitions
    RESPONSE_OK = 200,
    //4xx: Errors from client
    ERROR_BAD_REQUEST = 400,
    ERROR_INCORRECT_NAME = 401,
    ERROR_NAME_TAKEN = 402,
    ERROR_PLAYER_NOT_FOUND = 403,
    ERROR_PLAYER_OCCUPIED = 404,
    ERROR_BAD_SLOT = 405,
    ERROR_NOT_TURN = 406,
    ERROR_INVALID_COMMAND = 407,
    ERROR_COMMAND_OUT_OF_CONTEXT = 408,
    ERROR_CONNECTION_LOST = 409,
    //5xx: Errors from server
    ERROR_SERVER_ERROR = 500,
};

enum context_t {
    MAIN_MENU,
    LOBBY_MENU,
    SEARCH_BY_IP,
    SEARCH_LOCAL_SERVERS,
    READY_TO_CONNECT,
    CONNECT,
    SEND_INVITE_MENU,
    INVITE_FROM_MENU,
    SEND_RANDOMINVITE_MENU,
    LOGOUT_CONTEXT,
    IN_A_GAME,
    READY_TO_PLAY,
    CLOSE_PROGRAM
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

class T3PCommand {
    public:
        T3PCommand();
        string command;
        list<string> dataList; 
        bool isNewCommand = false;
        void clear();
};

enum tcpcommand_t {
    INVITEFROM,
    INVITATIONTIMEOUT,
    ACCEPT,
    DECLINE,
    TURNPLAY,
    TURNWAIT,
    MATCHEND
};

