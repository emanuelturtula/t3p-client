#include <string>

using namespace std;

class Server {
    private:
        string ip = "127.0.0.1";
        string name;
        size_t players_online = 0;
        size_t players_occupied = 0;
        bool selected = false;

    public:
        Server(string name);
        
        // Getters 
        string getIp();
        string getName();
        size_t getPlayersOnline();
        size_t getPlayersOccupied();
        bool isSelected();

        // Setters
        void setIp(string ip);
        void setName(string name);
        void setPlayersOnline(size_t players);
        void setPlayersOccupied(size_t players);
        void setSelected(bool selection);
};