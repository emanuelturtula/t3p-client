#include "../headers/tests.h"
#include "../headers/types.h"
#include "../headers/menus.h" 

status_t run_test_udp();
status_t run_test_menus();


status_t run_tests()
{
    #ifdef DEBUG_UDP
        return run_test_udp();
    #endif
    #ifdef DEBUG_MENUS
        return run_test_menus();
    #endif
}

status_t run_test_udp()
{
    status_t status;
    list<T3PResponse> t3pResponseList;
    T3PResponse t3pResponse;
    string ip = "172.23.134.149";
    if ((status = send_discover_broadcast(&t3pResponseList)) != STATUS_OK)
        return status;
    if ((status = send_discover(ip, &t3pResponse)) != STATUS_OK)
        return status;
    return status;
}

status_t run_test_menus()
{
    status_t status;
    context_t context;
    Server testServer;
    // if ((status = main_menu(&context)) != STATUS_OK)
    //     return status;
    if ((status = search_local_servers_menu(&context, &testServer)) != STATUS_OK)
        return status;
    return status;
}