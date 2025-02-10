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
#define GET_PAYLOAD_END_INDEX(PAYLOAD_LENGTH) (PAYLOAD_LENGTH + 3)

typedef enum {
    SEARCHING_START_BYTE        = 0,
    CHECKING_FIRST_LENGTH       = 1,
    CHECKING_SECOND_LENGTH      = 2,
    CHECKING_SECOND_START_BYTE  = 3,
    CHECKING_CONTROL_BYTE       = 4,
    CHECKING_BAOS_PAYLOAD       = 5,
    CHECKING_CHECKSUM           = 6,
    CHECKING_END_BYTE           = 7,
    RECEPTION_COMPLETE          = 8,
} STATES;

typedef struct {
    STATES currentState;
    DWORD currentInputIndex;
    DWORD currentOutputIndex;
    unsigned char payloadLength;
    int checksumSum;
    bool doStartBytesMatch;
    bool doLengthBytesMatch;
    bool doesChecksumMatch;
    bool isEndByteFound;
} ReaderInfo;

STATES readBuffer(const unsigned char* pBuff, const DWORD bytesRead, unsigned char* destBuff, ReaderInfo* ri);
void printBuff(const unsigned char* pBuff, DWORD buffLen);
DWORD WINAPI InputThread(LPVOID lpParameter);