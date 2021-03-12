#include <string>

using namespace std;

class Server {
    private:
        string ip;
        size_t players_online = 0;
        size_t players_occupied = 0;
        bool selected = false;

    public:
        Server(string ip);
        
        // Getters 
        string getIp();
        size_t getPlayersOnline();
        size_t getPlayersOccupied();
        bool isSelected();

        // Setters
        void setIp(string ip);
        void setPlayersOnline(size_t players);
        void setPlayersOccupied(size_t players);
        void setSelected(bool selection);
};