#pragma once

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <vector>

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
    ERROR_POLL_DETECTED_0_EVENTS,

    ERROR_NOT_RECIVIED_INVTEFROM,
    ERROR_READY_TO_PLAY_MATCH_NOT_SET_FROM_SERVER,


    ERROR_NULL_POINTER,

    // This next status_t indicates number of status_t errors that exist in the program:
    // Currently, we have 20 possible errors:
    NUMBER_OF_INTERNAL_ERRORS_DEFINED_IN_CLIENT_PROGRAM, 

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
    SEARCH_LOCAL_SERVERS_MENU,
    SEARCH_BY_IP_MENU,
    FAST_CONNECT_MENU,
    READY_TO_CONNECT,
    CONNECT,
    LOBBY_MENU,
    INVITE_MENU,
    INVITE_FROM_MENU,
    RANDOMINVITE_MENU,
    LOGOUT,
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

class T3PServerMessages{
    private:
        string name;
        list<string> dataList;
    public:
        T3PServerMessages();
        void clear();
        void addData(string data);
        status_t parse_buffer(string dataStream);
        status_t setName(string name);
        string getName();
};

class Slot {
    public:
        Slot();
        void clear();
        bool available;
};

enum MatchSlot {
    CIRCLE,
    CROSS,
    EMPTY
};

enum MatchStatus{
    TIMEOUT,
    NORMAL,
    ENDED
};

class MatchInfo {
    private:
    vector<MatchSlot> slots;
    
    public:
        MatchInfo();
        MatchSlot playerSymbol;
        string circlePlayer = "";
        string crossPlayer = "";
        void printSlots();
        bool playAsCirle = false;
        bool myTurn;
        void clearSlots();
        void setSlots(string circleSlots, string crossSlots);
        vector<MatchSlot> getSlots();
        void printSlotsNumpad();
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

extern map<string, status_t> T3PStatusCodeMapper;
extern map<status_t,string> DICTIONARY_InternalServerErrorsMessages;

class ErrorHandler {
    public:
        ErrorHandler();
        void handle_error(status_t status,context_t *context, int socket);
};


