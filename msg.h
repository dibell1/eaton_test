#pragma once

#include <stdint.h>

constexpr int NAME_SIZE=16;
constexpr int KEY=0xab;

using SeqId = int32_t;

#pragma pack(push)  
#pragma pack(1) 

struct MsgData
{
    uint8_t name[NAME_SIZE];
    SeqId seqId;
};

struct Msg
{
    MsgData data;
    uint8_t pwd;
};

#pragma pack(pop)
