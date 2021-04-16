#include "../headers/main.h"
#include "../headers/types.h"
#include "../headers/t3p_client.h"
#include "../headers/tests.h"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

map<string, int> config = {
    {"TCP_PORT", 0},
    {"UDP_PORT", 0},
    {"CONNECTING_TIMEOUT", 0}
};

bool loadConfig();

int main()
{
    #if defined(DEBUG_UDP) || defined(DEBUG_MENUS) || defined(DEBUG_TCP) || defined(DEBUG_MATCH)
        status_t status;
        if ((status = run_tests()) != STATUS_OK){
            cout << "Ending with status_t = " << status << "\n";
            return EXIT_FAILURE;
        }
        cout << "Ending SUCCESSFULLY with status_t = " << status << "\n";
        return EXIT_SUCCESS;
    #else
        status_t status;
        if (!loadConfig())
            return EXIT_FAILURE;
        if ((status = t3p_client()) != STATUS_OK)
            return EXIT_FAILURE;
        return EXIT_SUCCESS;
    #endif
}

bool loadConfig()
{
    ifstream fin("config.txt");
    string line;
    while (getline(fin, line)) 
    {
        istringstream sin(line.substr(line.find("=") + 1));
        for (auto const& pair : config)
        {
            if (line.find(pair.first) != -1) 
            {
                if (pair.second == 0)
                    sin >> config[pair.first];
                else
                {
                    cerr << "Error loading configuration file. Key is repeated" << endl;
                    return false;
                }   
            }
        }
    }

    for (auto const& pair : config)
    {
        if (pair.second == 0)
        {
            cerr << "Error loading configuration file. Missing value: " << pair.first << endl;
            return false;
        }
    }
    
    return true;
}