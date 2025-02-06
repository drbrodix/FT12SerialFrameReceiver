#include "SerialReader.h"

#include <string.h>

void printBuff(const unsigned char* pBuff, int buffLen)
{
    for (int i = 0; i < buffLen; i++)
    {
        printf("%x, ", pBuff[i]);
    }
    printf("\n");
}

int lookForFT12Start(const unsigned char* pBuff, int buffLen, int* patternCheckingStartIndex)
{
    int startByteIndex = -1;

    for (; *patternCheckingStartIndex < buffLen; (*patternCheckingStartIndex)++)
    {
        if (pBuff[*patternCheckingStartIndex] == FT12_START_BYTE)
        {
            startByteIndex = *patternCheckingStartIndex;
            break;
        }
    }
    
    return startByteIndex;
}

int checkPattern(const unsigned char* pBuff, int startByteIndex)
{
    bool isHeaderOk     = false;
    bool isEndByteFound = false;

                // Check matching start bytes
    isHeaderOk =((pBuff[startByteIndex]     == pBuff[startByteIndex + 3]) &&
                // Check matching length bytes
                (pBuff[startByteIndex + 1]  == pBuff[startByteIndex + 2])
                );

    const int END_BYTE_INDEX = FIND_FT12_END_BYTE(startByteIndex, pBuff[startByteIndex + 1]);
    isEndByteFound = pBuff[END_BYTE_INDEX] == FT12_END_BYTE;

    return ((isHeaderOk && isEndByteFound) ? END_BYTE_INDEX : -1);
}

bool checkChecksum(const unsigned char* pBuff, int startByteIndex, int endByteIndex)
{
    const int CONTROL_BYTE_INDEX    = startByteIndex + 4;
    const int CHECKSUM_BYTE_INDEX   = endByteIndex - 1;
    unsigned int sum = 0;

    for (int i = CONTROL_BYTE_INDEX; i < CHECKSUM_BYTE_INDEX; i++)
    {
        sum += pBuff[i];
    }
    return ((sum % 256) == pBuff[CHECKSUM_BYTE_INDEX]);
}

STATES readBuffer(const unsigned char* pBuff, const int buffLen, unsigned char* destBuff)
{
    STATES currentState = CHECKING;
    int startByteIndex = -1;
    int endByteStartIndex = -1;
    int patternCheckingStartIndex = 0;

    while (patternCheckingStartIndex < buffLen)
    {
        startByteIndex = lookForFT12Start(pBuff, buffLen, &patternCheckingStartIndex);

        if(startByteIndex < 0)
        {
            currentState = START_BYTE_NOT_FOUND;
            return currentState;
        }

        endByteStartIndex = checkPattern(pBuff, startByteIndex);

        if(endByteStartIndex >= 0)
            break;

        patternCheckingStartIndex++;
    }

    if(endByteStartIndex < 0)
    {
        currentState = PATTERN_MISMATCH;
        return currentState;
    }

    if(!checkChecksum(pBuff, startByteIndex, endByteStartIndex))
    {
        currentState = CHECKSUM_ERROR;
        return currentState;
    }
    currentState = FRAME_OK;
    memcpy_s(destBuff, FT12_ARRAY_LENGTH, pBuff + startByteIndex, GET_FT12_LENGTH(startByteIndex, endByteStartIndex));
    return currentState;
}

int main(int argc, char const *argv[])
{
    // Test input buffer array
    /*const unsigned char testPack[INPUT_ARRAY_LENGTH] =   {0x68, 0x68, 0x16, 0x68,
                                                            0x68, 0x68, 0x0c, 0x0c,
                                                            0x68, 0x73, 0xf0, 0x06,
                                                            0x00, 0x03, 0x00, 0x01,
                                                            0x00, 0x03, 0x03, 0x01,
                                                            0x01, 0x75, 0x16, 0x0e};*/

    // Buffer array for storing incoming FT 1.2 frame
    unsigned char ft12Buffer[FT12_ARRAY_LENGTH] = {};
    unsigned char buffer[INPUT_ARRAY_LENGTH] = {};
    HANDLE serialHandle;
    COMMTIMEOUTS timeouts;
    DWORD read;
    DCB port;
    const char commName[128] = "\\\\.\\COM3";

    // open the comm port.
    serialHandle = CreateFile(commName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if(INVALID_HANDLE_VALUE == serialHandle) {
        printf("Couldn't open port");
        return 1;
    }

    // get the current DCB, and adjust a few bits to our liking.
    memset(&port, 0, sizeof(port));
    port.DCBlength = sizeof(port);
    if(!GetCommState(serialHandle, &port))
        printf("Couldn't get comm state");

    port.BaudRate = CBR_19200;
    port.ByteSize = 8;
    port.StopBits = ONESTOPBIT;
    port.Parity   = EVENPARITY;
    if(!SetCommState(serialHandle, &port))
        printf("Couldn't adjust port settings");

    // set short timeouts on the comm port.
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.WriteTotalTimeoutMultiplier = 1;
    timeouts.WriteTotalTimeoutConstant = 1;
    if(!SetCommTimeouts(serialHandle, &timeouts))
        printf("Couldn't set port timeouts.");

    while(true)
    {
        // check for data on port and display it on screen.
        ReadFile(serialHandle, buffer, sizeof(buffer), &read, NULL);
        if(read)
        {
            printf("%d\n", readBuffer(buffer, sizeof(buffer), ft12Buffer));

            printBuff(ft12Buffer, FT12_ARRAY_LENGTH);

            /*printBuff(buffer, sizeof(buffer));*/
        }
    }

    return 0;
}
