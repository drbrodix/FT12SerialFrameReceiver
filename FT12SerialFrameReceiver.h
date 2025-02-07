#pragma once

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <Windows.h>

#define READ_TIMEOUT        500
#define INPUT_ARRAY_LENGTH  200
#define FT12_ARRAY_LENGTH   250
#define FT12_START_BYTE     0x68
#define FT12_END_BYTE       0x16
#define FIND_FT12_END_BYTE(START_BYTE_INDEX, PAYLOAD_LENGTH) (START_BYTE_INDEX + 5 + PAYLOAD_LENGTH)
#define GET_FT12_LENGTH(START_BYTE_INDEX, END_BYTE_INDEX) ((END_BYTE_INDEX - START_BYTE_INDEX) + 1)

typedef enum {
    SEARCHING_START_BYTE        = 0,
    CHECKING_FIRST_LENGTH       = 1,
    CHECKING_SECOND_LENGTH      = 2,
    CHECKING_SECOND_START_BYTE  = 3,
    CHECKING_CONTROL_BYTE       = 4,
    HEADER_FOUND                = 5,
    CHECKSUM_ERROR              = 6,
    FRAME_OK                    = 7
} STATES;

typedef struct {
    STATES currentState;
    DWORD currentInputIndex;
    DWORD currentOutputIndex;
    unsigned char payloadLength;
} ReaderInfo;

STATES readBuffer(const unsigned char* pBuff, const DWORD bytesRead, unsigned char* destBuff, ReaderInfo* ri);
void printBuff(const unsigned char* pBuff, DWORD buffLen);
DWORD lookForFT12Start(const unsigned char* pBuff, DWORD buffLen, DWORD* patternCheckingStartIndex);
DWORD checkPattern(const unsigned char* pBuff, DWORD startByteIndex);
bool checkChecksum(const unsigned char* pBuff, DWORD startByteIndex, DWORD endByteIndex);
DWORD WINAPI InputThread(LPVOID lpParameter);