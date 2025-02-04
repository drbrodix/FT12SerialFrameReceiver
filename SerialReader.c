#include "SerialReader.h"

#define ARRAY_LENGTH 200

short lookForFTStart(const char* pBuff)
{
    
}

void readBuff(const char* pBuff)
{
    lookForFTStart(pBuff);
}

int main(int argc, char const *argv[])
{
    const char testPack[ARRAY_LENGTH] ={0x68, 0x0c, 0x0c, 0x68,
                                        0x73, 0xf0, 0x06, 0x00,
                                        0x03, 0x00, 0x01, 0x00,
                                        0x03, 0x03, 0x01, 0x01,
                                        0x75, 0x16};

    readBuff(testPack);
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