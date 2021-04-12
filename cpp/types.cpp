#include "../headers/types.h"
#include "../headers/tcp.h"
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


/*Prints Slots in next format example:
 o | - | x 
-----------     
   | x |   
-----------
   | x |   
*/
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
    {"200", STATUS_OK},
    {"400", ERROR_BAD_REQUEST},
    {"405", ERROR_BAD_SLOT},
    {"406", ERROR_NOT_TURN},
    {"408", ERROR_COMMAND_OUT_OF_CONTEXT},
    {"500", ERROR_SERVER_ERROR}
};

map<status_t,string> DICTIONARY_InternalServerErrorsMessages = {
    {STATUS_OK, "STATUS_OK"},
    {ERROR_SOCKET_CREATION, "ERROR_SOCKET_CREATION"}, 
    {ERROR_SOCKET_CONFIGURATION, "ERROR_SOCKET_CONFIGURATION"},
    {ERROR_SOCKET_LISTENING,"ERROR_SOCKET_LISTENING"},
    {ERROR_SOCKET_READING,"ERROR_SOCKET_READING"},
    {ERROR_SOCKET_BINDING,"ERROR_SOCKET_BINDING"},
    {ERROR_SENDING_MESSAGE,"ERROR_SENDING_MESSAGE"},
    {ERROR_RECEIVING_MESSAGE,"ERROR_RECEIVING_MESSAGE"},
    {ERROR_NO_SERVERS_ONLINE,"ERROR_NO_SERVERS_ONLINE"},
    {ERROR_BAD_MESSAGE_FORMAT,"ERROR_BAD_MESSAGE_FORMAT"},
    {ERROR_CONNECTING,"ERROR_CONNECTING"},
    {ERROR_LOGIN,"ERROR_LOGIN"},
    {ERROR_STATUS_MESSAGE,"ERROR_STATUS_MESSAGE"},
    {ERROR_BAD_PLAYER_NAME,"ERROR_BAD_PLAYER_NAME"},
    {ERROR_OUT_OF_CONTEXT,"ERROR_OUT_OF_CONTEXT"},
    {ERROR_UNEXPECTED_EVENT_POLL_TCP_INVITATION_FROM,"ERROR_UNEXPECTED_EVENT_POLL_TCP_INVITATION_FROM"},
    {ERROR_POLL_DETECTED_0_EVENTS,"ERROR_POLL_DETECTED_0_EVENTS"},
    {ERROR_NOT_RECIVIED_INVTEFROM,"ERROR_NOT_RECIVIED_INVTEFROM"},
    {ERROR_READY_TO_PLAY_MATCH_NOT_SET_FROM_SERVER,"ERROR_READY_TO_PLAY_MATCH_NOT_SET_FROM_SERVER"},
    {ERROR_NULL_POINTER,"ERROR_NULL_POINTER"},
    ////////////////////////7
        /*******************************/
        // RFC Responses:
        //1xx: Informative responses
    {INFO_NO_PLAYERS_AVAILABLE,"INFO_NO_PLAYERS_AVAILABLE"},
        //2xx: Correct petitions
    {RESPONSE_OK,"RESPONSE_OK"},
        //4xx: Errors from client
    {ERROR_BAD_REQUEST,"ERROR_BAD_REQUEST"},
    {ERROR_INCORRECT_NAME,"ERROR_INCORRECT_NAME"},
    {ERROR_NAME_TAKEN,"ERROR_NAME_TAKEN"},
    {ERROR_PLAYER_NOT_FOUND,"ERROR_PLAYER_NOT_FOUND"},
    {ERROR_PLAYER_OCCUPIED,"ERROR_PLAYER_OCCUPIED"},
    {ERROR_BAD_SLOT,"ERROR_BAD_SLOT"},
    {ERROR_NOT_TURN,"ERROR_NOT_TURN"},
    {ERROR_INVALID_COMMAND,"ERROR_INVALID_COMMAND"},
    {ERROR_COMMAND_OUT_OF_CONTEXT,"ERROR_COMMAND_OUT_OF_CONTEXT"},
    {ERROR_CONNECTION_LOST,"ERROR_CONNECTION_LOST"},
        //5xx: Errors from server
    {ERROR_SERVER_ERROR,"ERROR_SERVER_ERROR"}
};

ErrorHandler:: ErrorHandler(){}


void ErrorHandler:: handle_error(status_t status,context_t *context, int socket){
    
    status_t st;
    cout << endl;

    if (status < NUMBER_OF_INTERNAL_ERRORS_DEFINED_IN_CLIENT_PROGRAM){


        cout << "\033[1;31mError in client program: \033[0m" << "statusID = " << to_string(status) << endl;
        cout << "Error Message: " << DICTIONARY_InternalServerErrorsMessages[status] << endl << endl;
        cout << "Returning to LOBBY" << endl << endl;
        (*context) = LOBBY_MENU;

    }else if ( (((*context) == TURNPLAY) || ((*context) == TURNWAIT)) && status == ERROR_BAD_MESSAGE_FORMAT){ // In this case, it is a RFC ERROR from server and we have to notify it.
  
        cout << "\033[1;31mError from SERVER: \033[0m" << "statusID = " << to_string(status) << endl;
        cout << "Error Message: " << DICTIONARY_InternalServerErrorsMessages[status] << endl << endl;
        cout << "Notifiying server about error" << endl;
        cout << "This is an error related to T3P protocol. Please refer to documentation for information.";
        cout << "Logging out from server" << endl << endl;

        (*context) == LOGOUT_CONTEXT;

        send_tcp_message(socket, "400|Bad Request \r\n \r\n");

    
    } else { // We receive have an unexpected error related to the RFC:

        cout << "\033[1;31mError: \033[0m" << "statusID = " << to_string(status) << endl;
        cout << "Error Message: " << DICTIONARY_InternalServerErrorsMessages[status] << endl << endl;
        cout << "This is an error related to T3P protocol. Please refer to documentation for information.";
        cout << "Logging out from server" << endl << endl;

        (*context) == LOGOUT_CONTEXT;

    }
}
