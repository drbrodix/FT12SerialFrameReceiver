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
// Macro to calculate the index of the FT 1.2 end byte
#define FIND_FT12_END_BYTE(START_BYTE_INDEX, PAYLOAD_LENGTH) (START_BYTE_INDEX + 5 + PAYLOAD_LENGTH)
// Macro to calculate the length of the entire frame
#define GET_FT12_LENGTH(START_BYTE_INDEX, END_BYTE_INDEX) ((END_BYTE_INDEX - START_BYTE_INDEX) + 1)

// Enumerator of the used states of the state machine
typedef enum {
    SEARCHING_START_BYTE        = 0,
    CHECKING_FIRST_LENGTH       = 1,
    CHECKING_SECOND_LENGTH      = 2,
    CHECKING_SECOND_START_BYTE  = 3,
    HEADER_FOUND                = 4
} STATES;

// Structure to keep track of necessary infos
// related to the buffer analysis process
typedef struct {
    STATES currentState;
    DWORD currentIndex;
    DWORD currentRoundIndex;
    DWORD startByteIndex;
    DWORD endByteIndex;
    unsigned char payloadLength;
    bool resetRI;
} ReaderInfo;

// Creates serial handle
HANDLE createHandle(LPCSTR fileName);

// Configures serial handle
WINBOOL configPort(HANDLE hSerial);

// Configures serial handle timeouts
WINBOOL configTimeouts(HANDLE hSerial);

// Handles analysis of buffer. Returns current machine
// state if there's no more data in buffer to read.
STATES stateMachine(unsigned char* pBuff, DWORD bytesRead, ReaderInfo* ri);

// Prints a specified range in buffer
void printBuff(const unsigned char* pBuff, DWORD startIndex, DWORD endIndex);

// A function waiting for user input, passed to a separate thread.
// Necessary for the main thread to be able to keep
// reading from the serial bus, while also giving the
// user the chance to terminate the application.
DWORD WINAPI InputThread(LPVOID lpParameter);