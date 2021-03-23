#include "server.h"

using namespace std;

int main()
{
    thread thread_udp(&udp_thread);
    thread thread_tcp(&tcp_thread);
    thread_udp.join();
    thread_tcp.join();
    return EXIT_SUCCESS;
}