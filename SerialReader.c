#include "SerialReader.h"

#define ARRAY_LENGTH 200
#define FT12_STARTBYTE 0x68

int lookForFTStart(const char* pBuff, int buffLen)
{
    int startByteIndex = -1;

    for (int i = 0; i < buffLen; i++)
    {
        if (pBuff[i] == FT12_STARTBYTE)
            startByteIndex = i;
    }
    
    return startByteIndex;
}

bool checkPattern(const char* pBuff, int buffLen, int startByteIndex)
{
    return (pBuff[startByteIndex]       == pBuff[startByteIndex + 3] &&
            pBuff[startByteIndex + 1]   == pBuff[startByteIndex + 2]
            );
}

int readBuff(const char* pBuff, int buffLen)
{
    int startByteIndex = lookForFTStart(pBuff, buffLen);

    if(startByteIndex < 0)
        return -1;
    else
    {
        return checkPattern(pBuff, buffLen, startByteIndex);
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
    const char testPack[ARRAY_LENGTH] ={0x00, 0x68, 0x0c, 0x0c,
                                        0x68, 0x73, 0xf0, 0x06,
                                        0x00, 0x03, 0x00, 0x01,
                                        0x00, 0x03, 0x03, 0x01,
                                        0x01, 0x75, 0x16, 0x00};

    // printBuff(testPack, ARRAY_LENGTH);
    
    printf("%d", readBuff(testPack, ARRAY_LENGTH));
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