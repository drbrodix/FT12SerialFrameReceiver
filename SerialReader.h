#pragma once

#include <stdbool.h>
#include <stdio.h>
// #include <Windows.h>

void printBuff(const char* pBuff, int buffLen);
int lookForFTStart(const char* pBuff, int buffLen);
bool checkPattern(const char* pBuff, int startByteIndex);
int findFt12Frame(const char* pBuff, int buffLen, int* destBuff);