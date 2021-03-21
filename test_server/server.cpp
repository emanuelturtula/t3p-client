/* Creates a datagram server.  The port

   number is passed as an argument.  This

   server runs forever */

 

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <iostream>

using namespace std;

void error(const char *msg);

void error(const char *msg)
{
    perror(msg);
}

int main()
{
    int sock, length, n;
    socklen_t fromlen;
    string input;
    struct sockaddr_in server = {0};
    struct sockaddr_in from = {0};
    char buf[1024];

    const char * cstr1 = "200|OK|127.0.0.1 \r\n \r\n";
    const char * cstr2 = "200|OK|172.23.134.149 \r\n \r\n";
    const char * cstr3 = "200|OK|edturtu|grizz \r\ngferrari|nico \r\n \r\n";
    const char * cstr4 = "200|OK|edturtu|grizz \r\n \r\n \r\n";
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) 
        error("Opening socket");
    length = sizeof(server);

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(2001);


    if (bind(sock,(struct sockaddr *)&server,length)<0)
        error("binding");


    fromlen = sizeof(struct sockaddr_in);
    printf("Waiting for message... \n\n");
    while (1) 
    {
        memset(buf, 0, strlen(buf));
        n = recvfrom(sock, buf, 1024, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0) 
            error("recvfrom");
        if (n > 0)
        {
            if (strcmp(buf, "DISCOVERBROADCAST \r\n \r\n") == 0)
            {
                cout << "\nReceived DISCOVERBROADCAST." << endl;
                
                n = sendto(sock, cstr2, strlen(cstr2), 0, (struct sockaddr *)&from, fromlen);
                if (n < 0) 
                    error("sendto");
            }
            else if (strcmp(buf, "DISCOVER|172.23.134.149 \r\n \r\n") == 0)
            {
                cout << "\nReceived DISCOVER." << endl;
                
                n = sendto(sock, cstr4, strlen(cstr4), 0, (struct sockaddr *)&from, fromlen);
                if (n < 0) 
                    error("sendto");
            }
        }
   }
}