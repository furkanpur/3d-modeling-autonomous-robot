// to communicate between Jetson TK1 and computer

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

void client_send_point(char *buffer, int sockfd, std::string data)
{
    int n;

    bzero(buffer, 256);

    memcpy(buffer,data.c_str(), strlen(data.c_str()));

    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0)
        printf("Error writing to socket");

    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);

    if (n < 0)
        printf("Error reading from socket");

    //printf("%s\n", buffer);
}
