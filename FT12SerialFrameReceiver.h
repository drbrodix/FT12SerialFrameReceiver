#pragma once

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <Windows.h>

#define READ_TIMEOUT        500
#define INPUT_ARRAY_LENGTH  250
#define FT12_ARRAY_LENGTH   250
#define FT12_START_BYTE     0x68
#define FT12_END_BYTE       0x16
#define FIND_FT12_END_BYTE(START_BYTE_INDEX, PAYLOAD_LENGTH) (START_BYTE_INDEX + 5 + PAYLOAD_LENGTH)
#define GET_FT12_LENGTH(START_BYTE_INDEX, END_BYTE_INDEX) ((END_BYTE_INDEX - START_BYTE_INDEX) + 1)
#define GET_PAYLOAD_END_INDEX(PAYLOAD_LENGTH) (PAYLOAD_LENGTH + 3)

typedef enum {
    SEARCHING_START_BYTE        = 0,
    CHECKING_FIRST_LENGTH       = 1,
    CHECKING_SECOND_LENGTH      = 2,
    CHECKING_SECOND_START_BYTE  = 3,
    HEADER_FOUND                = 4
} STATES;

typedef struct {
    STATES currentState;
    DWORD readerIndex;
    DWORD startByteIndex;
    DWORD endByteIndex;
    unsigned char payloadLength;
} ReaderInfo;

HANDLE createHandle(LPCSTR fileName);
WINBOOL configPort(HANDLE hSerial);
WINBOOL configTimeouts(HANDLE hSerial);
void readBuffer(unsigned char* pBuff, DWORD bytesRead, unsigned char* destBuff, ReaderInfo* ri);
STATES stateMachine(unsigned char* pBuff, DWORD bytesRead, ReaderInfo* ri);
void printBuff(const unsigned char* pBuff, DWORD startIndex, DWORD endIndex);
DWORD WINAPI InputThread(LPVOID lpParameter);