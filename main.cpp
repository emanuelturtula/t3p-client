#include <iostream>
#include <thread>

#include "threads/io_thread.h"
#include "threads/udp_thread.h"
#include "threads/tcp_thread.h"

#include "test_thread.h"
#include "classes/Context.h"
#include "types.h"

using namespace std;

int main()
{
    thread t1(read_context);
    t1.join();
    return 0;
}