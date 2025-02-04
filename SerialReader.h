#include <stdbool.h>
#include <stdio.h>
#include <Windows.h>

void printBuff(const char* pBuff, int buffLen);
int lookForFTStart(const char* pBuff, int buffLen);
bool checkPattern(const char* pBuff, int buffLen, int startByteIndex);
int readBuff(const char* pBuff, int buffLen);