#pragma once

#include "types.h"
#include <list>

#define UDP_PORT 2001
#define BUFFER_SIZE 1024

using namespace std;

status_t send_discover_broadcast(list<T3PResponse> *t3pResponseList);
status_t send_discover(string ip, T3PResponse *t3pResponse);