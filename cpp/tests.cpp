#include "../headers/tests.h"
#include "../headers/types.h"
#include "../headers/menus.h"
#include "../headers/udp.h"
#include "../headers/tcp.h"
#include <thread>

using namespace std;

status_t run_test_udp();
status_t run_test_tcp();
status_t run_test_menus();
status_t test_mark_slots();

status_t run_tests()
{
    #ifdef DEBUG_UDP
        return run_test_udp();
    #endif
    #ifdef DEBUG_MENUS
        return run_test_menus();
    #endif
    #ifdef DEBUG_TCP
        return run_test_tcp();
    #endif
    #ifdef DEBUG_MATCH
        return test_mark_slots();
    #endif
    return STATUS_OK;
}

status_t run_test_udp()
{
    status_t status;
    list<T3PResponse> t3pResponseList;
    T3PResponse t3pResponse;
    string ip = "192.168.0.40";
    // if ((status = send_discover_broadcast(&t3pResponseList)) != STATUS_OK)
    //     return status;
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
    // if ((status = search_local_servers_menu(&context, &testServer)) != STATUS_OK)
    //     return status;
    // context = SEARCH_BY_IP;
    // if ((status = search_by_ip_menu(&context, &testServer)) != STATUS_OK)
    //     return status;
    context = READY_TO_CONNECT;
    testServer.ip = "192.168.0.40";
    testServer.playersAvailable.push_back("edturtu");
    testServer.playersAvailable.push_back("gferrari");
    testServer.playersOccupied.push_back("nico");
    testServer.playersOccupied.push_back("pepe");
    if ((status = connect_menu(&context, testServer)) != STATUS_OK)
         return status;
    return status;
}

status_t test_mark_slots()
{
    MatchInfo match;
    match.setSlots("123", "456");
    return STATUS_OK;
}
// status_t run_test_tcp()
// {
//     status_t status;
//     Server testServer;
//     int sockfd;
//     string player_name = "edturtu";
//     testServer.ip = "127.0.0.1";
//     bool accept = false;
//     bool response;
//     if ((status = login(testServer, player_name, &sockfd)) != STATUS_OK)
//         return status;
//     // thread heartbeat_thd(heartbeat_thread, sockfd);
//     // if ((status = logout(&sockfd)) != STATUS_OK)
//     //     return status;
//     // heartbeat_thd.join();
//     cout << "1. Sending invitation to edturtu = true\n";
//     if ((status = invite(sockfd, "edturtu", &response)) != STATUS_OK)
//         return status;


//     cout << "1.2 wait_invitation_response from edturtu\n";
//     if ((status = wait_invitation_response(sockfd, &accept)) != STATUS_OK)
//         return status;  
//     cout << "1.3 invitation to edturtu result in: " << accept << ". Should be TRUE == 1\n";



//     cout << "3. Sending random_invite\n";
//     if ((status = random_invite(sockfd)) != STATUS_OK)
//         return status;


//     cout << "3.1 wait_invitation_response from random_invite\n";
//     if ((status = wait_invitation_response(sockfd, &accept)) != STATUS_OK)
//         return status;
//     cout << "3.3 random_invite result in: " << accept << ". Should be FALSE\n";


//     cout << "4. Sending invitation_response = false. (we do not receive answer for this case)\n";
//     if ((status = invitation_response(sockfd, &accept)) != STATUS_OK)
//         return status;  


//     return STATUS_OK;
// }