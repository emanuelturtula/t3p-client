#include "../headers/main.h"
#include "../headers/types.h"
#include "../headers/t3p_client.h"

#ifdef DEBUG_UDP
    #include <list>
    #include <iostream>
    #include <string>
    #include "../headers/udp.h"
#endif

using namespace std;

int main()
{
    #ifdef DEBUG_UDP
        list<T3PResponse> t3pResponseList;
        T3PResponse t3pResponseObj;
        string ip = "127.0.0.1";
        if (send_discover_broadcast(&t3pResponseList) != STATUS_OK)
            return EXIT_FAILURE;
        if (send_discover(ip, &t3pResponseObj) != STATUS_OK)
            return EXIT_FAILURE;
        if (send_discover_broadcast(&t3pResponseList) != STATUS_OK)
            return EXIT_FAILURE;
        cout << t3pResponseList.front().statusCode;
        return EXIT_SUCCESS;
    #endif
}