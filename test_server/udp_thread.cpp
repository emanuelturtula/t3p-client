#include "server.h"

using namespace std;

int respond(string buffer, int sockfd, struct sockaddr_in client);

void udp_thread()
{
    int sockfd;
    socklen_t clientlen = sizeof(struct sockaddr_in);
    string input;
    struct sockaddr_in server = {0};
    struct sockaddr_in client = {0};

    char buffer[UDP_BUFFER_SIZE];

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(UDP_PORT);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        write_stderr("UDP THREAD - ERROR CREATING SOCKET");
        return;
    }

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        close(sockfd);
        write_stderr("UDP THREAD - ERROR BINDING");
        return;
    }

    string message = "UDP THREAD - Waiting UDP MESSAGES on port ";
    message += to_string(UDP_PORT);

    write_stdout(message);

    while (1) 
    {
        memset(buffer, 0, strlen(buffer));
        if (recvfrom(sockfd, buffer, UDP_BUFFER_SIZE, 0, (struct sockaddr *)&client, &clientlen) < 0)
        {
            close(sockfd);
            write_stderr("UDP THREAD - ERROR RECEIVING MESSAGE");
            return; 
        }
        else
        {
            if (respond(string(buffer), sockfd, client) != 0)
            {
                close(sockfd);
                write_stderr("UDP THREAD - ERROR RESPONDING MESSAGE");
                return; 
            }
        }
   }
}

int respond(string buffer, int sockfd, struct sockaddr_in client)
{
    const char *discover_broadcast_response = "200|OK|127.0.0.1 \r\n \r\n";
    const char *discover_response = "200|OK|edturtu|grizz \r\ngferrari|nico \r\n \r\n";
    size_t clientlen = sizeof(struct sockaddr_in);

    if (buffer == "DISCOVERBROADCAST \r\n \r\n")
    {
        write_stdout("UDP THREAD - Received DISCOVERBROADCAST.");
        if (sendto(sockfd, discover_broadcast_response, strlen(discover_broadcast_response), 0, (struct sockaddr *)&client, clientlen) < 0)
            return -1;
    }
    else if (buffer == "DISCOVER \r\n \r\n")
    {
        write_stdout("UDP THREAD - Received DISCOVER.");
        if (sendto(sockfd, discover_response, strlen(discover_response), 0, (struct sockaddr *)&client, clientlen) < 0)
            return -1;
    }
    else 
    {
        write_stderr("UDP THREAD - Unrecognized command.");
    }
    return 0;
}