#include "SerialReader.h"

#define INPUT_ARRAY_LENGTH 200
#define FT12_ARRAY_LENGTH 250
#define FT12_START_BYTE 0x68
#define FT12_END_BYTE 0x16
#define FIND_FT12_END_BYTE(START_BYTE_INDEX, PAYLOAD_LENGTH) (START_BYTE_INDEX + 4 + PAYLOAD_LENGTH)

int lookForFTStart(const char* pBuff, int buffLen)
{
    int startByteIndex = -1;

    for (int i = 0; i < buffLen; i++)
    {
        if (pBuff[i] == FT12_START_BYTE)
        {
            startByteIndex = i;
            break;
        }
    }
    
    return startByteIndex;
}

bool checkPattern(const char* pBuff, int startByteIndex)
{
    bool isHeaderOk     = false;
    bool isEndByteFound = false;

    isHeaderOk =((pBuff[startByteIndex]       == pBuff[startByteIndex + 3]) &&
                (pBuff[startByteIndex + 1]   == pBuff[startByteIndex + 2])
                );

    isEndByteFound = pBuff[FIND_FT12_END_BYTE(startByteIndex, pBuff[startByteIndex + 1])];

    return isHeaderOk && isEndByteFound;
}

int findFt12Frame(const char* pBuff, int buffLen, int* destBuff)
{
    const int startByteIndex = lookForFTStart(pBuff, buffLen);

    if(startByteIndex < 0)
        return -1;
    else
    {
        return checkPattern(pBuff, startByteIndex);
    }
}

void printBuff(const char* pBuff, int buffLen)
{
    for (int i = 0; i < buffLen; i++)
    {
        printf("%x, ", pBuff[i]);
    }
}

int main(int argc, char const *argv[])
{
    // Test input buffer array
    const int testPack[INPUT_ARRAY_LENGTH]={0x00, 0x68, 0x0c, 0x0c,
                                            0x68, 0x73, 0xf0, 0x06,
                                            0x00, 0x03, 0x00, 0x01,
                                            0x00, 0x03, 0x03, 0x01,
                                            0x01, 0x75, 0x16, 0x00};

    // Buffer array for storing incoming FT 1.2 frame
    const int ft12Buffer[FT12_ARRAY_LENGTH] = {};

    // printBuff(ft12Buffer, FT12_ARRAY_LENGTH);
    
    printf("%d", findFt12Frame(testPack, INPUT_ARRAY_LENGTH, ft12Buffer));
    // const LPCWSTR COM_NAME = "COM3";

    // HANDLE serialHandle = CreateFile(
    //                                     COM_NAME,
    //                                     GENERIC_READ | GENERIC_WRITE,
    //                                     0,
    //                                     NULL,
    //                                     OPEN_EXISTING,
    //                                     FILE_ATTRIBUTE_NORMAL,
    //                                     NULL
    //                                 );

    return 0;
}