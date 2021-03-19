#include <string>
#include <list>

enum status_t {
    STATUS_OK
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