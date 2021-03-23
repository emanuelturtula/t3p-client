#pragma once

#include <string>
#include "types.h"

#define TCP_PORT 2000
#define BUFFER_SIZE 1024

using namespace std;

status_t login(Server server, string player_name, int *sockfd);
status_t logout(int *sockfd);
void heartbeat_thread(int sockfd);
