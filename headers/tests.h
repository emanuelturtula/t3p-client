#pragma once

#include <list>
#include <iostream>
#include <string>
#include <mutex>
#include <unistd.h>
#include "../headers/types.h"

//DEBUG SELECTION
//#define DEBUG_UDP
//#define DEBUG_MENUS
#define DEBUG_TCP



status_t run_tests();