#pragma once

#include <string>
#include "types.h"

#define TCP_PORT 2000

using namespace std;

status_t login(int *sockfd, string player_name, string ip);
