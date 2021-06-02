#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "msg.h"
#include "crc.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, port;
    struct sockaddr_in serv_addr = {0};
    struct hostent *server;
    Msg msg = { 0 };

    if (argc < 6) 
    {
       fprintf( stderr, "usage hostname port device_name seq_id password\n");
       exit(1);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        fprintf( stderr,"ERROR, no such host");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    { 
        error("ERROR opening socket");
    }

    port = atoi(argv[2]);
    if (port == 0)
    { 
        fprintf( stderr, "ERROR invalid port number");
        exit(1);
    }

    // get name
    const char* name = argv[3];
    size_t name_len = strlen(name);
    if ( name_len == 0 || name_len > ( NAME_SIZE-1 ) )
    {
        fprintf( stderr, "ERROR, wrong device name.");
        exit(1); 
    }

    // get password
    const char* pwd = argv[5];
    
    // get seq id
    SeqId seqId = atoi(argv[4]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    memcpy( msg.data.name, name, strlen(name) );
    msg.data.seqId = seqId;

    // compute crc hash of msg + pwd
    uint8_t buff[256];
    memcpy(buff, &msg, sizeof(MsgData));
    memcpy(buff + sizeof(MsgData), pwd, strlen(pwd));
    uint8_t crc = crcSlow( buff, sizeof(MsgData) + strlen(pwd));

    // encrypt crc hash
    msg.pwd = crc ^ KEY;

    int n = sendto(sockfd, &msg, sizeof(msg), 0, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (n < 0) 
    {
         error("ERROR writing to socket");
    }
    
    close(sockfd);
    return 0;
}