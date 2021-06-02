#pragma once

#include <stdint.h>

constexpr int NAME_SIZE=16;
constexpr int KEY=0xab;

using SeqId = int32_t;

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
