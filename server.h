#pragma once

using Name = std::string;

constexpr int INVALID_SOCKET = -1;

class UdpProvider 
{
    using OnData = std::function<void(const int8_t*, size_t)>;
public:
    UdpProvider & operator=(const UdpProvider&) = delete;
    UdpProvider(const UdpProvider&) = delete;
    UdpProvider() = default;

    UdpProvider(UdpProvider&& orig) 
    { 
        _sockfd = orig._sockfd;
        _abort = orig._abort; 
        orig._sockfd = INVALID_SOCKET; 
    };

    bool open(int port);
    void close();

    void start(const OnData&& onData);
    void stop();
    
private:
    bool _abort = false;
    int _sockfd = INVALID_SOCKET;
};

class Device
{
public:
    void incCounter() { _counter++; }
    SeqId getSeq() const { return _seq; }
    void setSeq( SeqId seq ) { _seq = seq; }

    uint getCounter() const { return _counter; }

private:
    uint _counter = 0;
    SeqId _seq = -1;
};

class DeviceRepo
{
    using DeviceCollection = std::unordered_map<Name, std::unique_ptr<Device>>;
public:
    void update( const Name& name, SeqId seqId, Value value );
    void print();

private:
     DeviceCollection _devices;
};

class Parser
{
public:
    Parser( const char *pwd) : _pwd( pwd ) {}

    bool parse( const int8_t* data, size_t len, Name& name, SeqId& seq, Value& value, size_t& proc_len );

private:
    std::string _pwd;
};