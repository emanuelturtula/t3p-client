#include "../headers/main.h"
#include "../headers/types.h"
#include "../headers/t3p_client.h"

#ifdef DEBUG_UDP
    #include <list>
    #include <iostream>
    #include "../headers/udp.h"
#endif

using namespace std;

int main()
{
    #ifdef DEBUG_UDP
        list<T3PResponse> t3pResponseList;
        if (send_discover_broadcast(&t3pResponseList) != STATUS_OK)
            return EXIT_FAILURE;
        
        return EXIT_SUCCESS;
    #endif
}