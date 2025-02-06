#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <Windows.h>

#define INPUT_ARRAY_LENGTH 200
#define FT12_ARRAY_LENGTH 250
#define FT12_START_BYTE 0x68
#define FT12_END_BYTE 0x16
#define FIND_FT12_END_BYTE(START_BYTE_INDEX, PAYLOAD_LENGTH) (START_BYTE_INDEX + 5 + PAYLOAD_LENGTH)
#define GET_FT12_LENGTH(START_BYTE_INDEX, END_BYTE_INDEX) ((END_BYTE_INDEX - START_BYTE_INDEX) + 1)

typedef enum {
    CHECKING                = 0,
    FRAME_OK                = 1,
    START_BYTE_NOT_FOUND    = 2,
    PATTERN_MISMATCH        = 3,
    CHECKSUM_ERROR          = 4,
} STATES;

STATES readBuffer(const unsigned char* pBuff, const int buffLen, unsigned char* destBuff);
void printBuff(const unsigned char* pBuff, int buffLen);
int lookForFT12Start(const unsigned char* pBuff, int buffLen, int* patternCheckingStartIndex);
int checkPattern(const unsigned char* pBuff, int startByteIndex);
bool checkChecksum(const unsigned char* pBuff, int startByteIndex, int endByteIndex);