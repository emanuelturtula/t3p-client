#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>

#define TCP_PORT 2000
#define TCP_BUFFER_SIZE 1024

#define UDP_PORT 2001
#define UDP_BUFFER_SIZE 1024

using namespace std;

void tcp_thread();
void udp_thread();
void write_stdout(string message); 
void write_stderr(string message);