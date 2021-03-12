#include <string>

using namespace std;

class Server {
    private:
        string ip;
        string name;
        size_t players_online;
        size_t players_occupied;

    public:
        Server(string name);

};