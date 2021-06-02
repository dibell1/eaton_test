/* The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <signal.h>
#include <cassert>
#include <fcntl.h>

#include "msg.h"
#include "crc.h"
#include "server.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

/*******************DataProvider*******************/

bool UdpProvider::open( int port )
{
    assert( _sockfd == INVALID_SOCKET );
    
    struct sockaddr_in serv_addr = { 0 };
    
    _sockfd =  socket(AF_INET, SOCK_DGRAM, 0);
    if (_sockfd < 0)
    { 
        return false;
    }

    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = INADDR_ANY;  
    serv_addr.sin_port = htons(port);

    if (fcntl(_sockfd, F_SETFL, fcntl(_sockfd, F_GETFL, 0) | O_NONBLOCK) == -1)
    {
        return false;
    }

    if (bind(_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    { 
       return false;
    }

    return true;
}

void UdpProvider::close()
{
    if ( _sockfd != INVALID_SOCKET )
    {
        ::close( _sockfd );
    }
}

void UdpProvider::start( const OnData&& onData )
{
    assert( _sockfd != INVALID_SOCKET );
    
    OnData _onData = std::move(onData);

    socklen_t clilen;
    int8_t buffer[256];
    struct sockaddr_in cli_addr = { 0 };
    socklen_t len = sizeof( cli_addr );
    
    struct timeval tv = { 0 };
    tv.tv_sec = 1;

    fd_set rfds;
    _abort = false;

    while(!_abort)
    {
        FD_ZERO(&rfds);
        FD_SET(_sockfd, &rfds);
        
        int ret = select(_sockfd + 1,  &rfds, NULL, NULL, &tv);

        if ( ret > 0)
        {
            int n = recvfrom(_sockfd, (int8_t *)buffer, ARRAY_SIZE(buffer), 0, ( struct sockaddr *) &cli_addr, &len);
            
            _onData( buffer, n );
        }
        else if (ret == -1 )
        {
            break;
        }
    }
}

void UdpProvider::stop()
{
    _abort = true;
}

/*******************Raw data parser*******************/

bool Parser::parse( const int8_t* data, size_t len, Name& name, SeqId& seq, size_t& proc_len )
{
    proc_len = 0;
    bool ret = false;

    if ( len >= sizeof(Msg) )
    {
        Msg* msg =(Msg*)data;
        
        // decrypt crc hash
        uint8_t received_pwd = msg->pwd;
        received_pwd ^= KEY;

        // compute crc hash
        uint8_t buff[256];
        memcpy(buff, &msg->data, sizeof(MsgData));
        memcpy(buff + sizeof(MsgData), _pwd.c_str(), _pwd.length());
        uint8_t counted_pwd = crcSlow( buff, sizeof(MsgData) +  _pwd.length());
        
        // compare
        if (received_pwd == counted_pwd)
        {
            name = (const char*)msg->data.name; 
            seq = msg->data.seqId;
            ret = true;
        }
        else
        {
            fprintf( stderr, "invalid password \n");
        }
    
        proc_len = sizeof(Msg);
    }

    return ret;
}

/*******************DeviceRepository*******************/

void DeviceRepo::update( const Name& name, SeqId seqId )
{
    Device* dev;
    DeviceCollection::const_iterator it = _devices.find(name);
    
    if ( it == _devices.end() )
    {
        auto ret = _devices.emplace( name, std::make_unique<Device>() );
        dev = ret.first->second.get();        
    }
    else
    {
        dev = it->second.get();
    }
    
    if ( seqId > dev->getSeq() )
    {
        dev->incCounter();
        dev->setSeq( seqId );
    }
    else
    {
        fprintf( stderr, "invalid seq number \n");
    }
}

void DeviceRepo::print()
{
    for (auto& it : _devices)
    {
        fprintf( stderr, "name: %s, count: %u\n", it.first.c_str(), it.second->getCounter());
    }
}

/*******************main*******************/

UdpProvider dataProvider;

int main(int argc, char *argv[])
{
    if (argc < 3) 
    {
         fprintf( stderr, "usage port password\n");
         exit(1);
    }

    int port = atoi(argv[1]);
    if (port == 0)
    {
         fprintf( stderr, "ERROR, invalid port number\n");
         exit(1);
    }

    auto sig_handler = []( int signo )
    {
        dataProvider.stop();
    };

    if (signal( SIGTERM, sig_handler ) == SIG_ERR)
    {
         fprintf( stderr, "ERROR, cannot set SIGSTOP handler\n");
         exit(1);
    }

    if (!dataProvider.open( port ))
    {
        perror("ERROR, socket error");
        exit(1);
    }

    const char* pwd = argv[2];
    Parser parser( pwd );
    DeviceRepo repo;

    UdpProvider prov = std::move(dataProvider);

    prov.start( [&parser,&repo] (const int8_t* data, size_t size) 
    {
        size_t proc;
        Name name;
        SeqId seq;
        
        do
        {
            bool valid = parser.parse( data, size, name, seq, proc );
            if ( valid )
            {
                repo.update( name, seq );
            }
         
            size -= proc;
            data += proc;

        } while ( proc > 0 );
    } ); 

    
    dataProvider.close();
    fprintf( stderr, "\nEND, device statistics:\n");
    repo.print();

    return 0; 
}