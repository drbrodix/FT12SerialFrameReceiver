#include "FT12SerialFrameReceiver.h"

void printBuff(const unsigned char* pBuff, DWORD buffLen) {
    for (DWORD i = 0; i < buffLen; i++)
    {
        printf("%x ", pBuff[i]);
    }
    printf("\n");
}

DWORD lookForFT12Start(const unsigned char* pBuff, DWORD buffLen, DWORD* patternCheckingStartIndex) {
    DWORD startByteIndex = -1;

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

DWORD checkPattern(const unsigned char* pBuff, DWORD startByteIndex) {
    bool isHeaderOk     = false;
    bool isEndByteFound = false;

                // Check matching start bytes
    isHeaderOk =((pBuff[startByteIndex]     == pBuff[startByteIndex + 3]) &&
                // Check matching length bytes
                (pBuff[startByteIndex + 1]  == pBuff[startByteIndex + 2])
                );

    const DWORD END_BYTE_INDEX = FIND_FT12_END_BYTE(startByteIndex, pBuff[startByteIndex + 1]);
    isEndByteFound = pBuff[END_BYTE_INDEX] == FT12_END_BYTE;

    return ((isHeaderOk && isEndByteFound) ? END_BYTE_INDEX : -1);
}

bool checkChecksum(const unsigned char* pBuff, DWORD startByteIndex, DWORD endByteIndex) {
    const DWORD CONTROL_BYTE_INDEX    = startByteIndex + 4;
    const DWORD CHECKSUM_BYTE_INDEX   = endByteIndex - 1;
    unsigned int sum = 0;

    for (DWORD i = CONTROL_BYTE_INDEX; i < CHECKSUM_BYTE_INDEX; i++)
    {
        sum += pBuff[i];
    }
    return ((sum % 256) == pBuff[CHECKSUM_BYTE_INDEX]);
}

STATES readBuffer(const unsigned char* pBuff, const DWORD bytesRead, unsigned char* destBuff, ReaderInfo* ri) {

    // typedef enum {
    //     SEARCHING_START_BYTE        = 0,
    //     CHECKING_FIRST_LENGTH       = 1,
    //     CHECKING_SECOND_LENGTH      = 2,
    //     CHECKING_SECOND_START_BYTE  = 3,
    //     CHECKING_CONTROL_BYTE       = 4,
    //     HEADER_FOUND                = 5,
    //     CHECKSUM_ERROR              = 6,
    //     FRAME_OK                    = 7
    // } STATES;
    //
    // typedef struct {
    //     STATES currentState;
    //     DWORD currentInputIndex;
    //     DWORD currentOutputIndex;
    //     unsigned char payloadLength;
    // } ReaderInfo;

    while (ri->currentInputIndex < bytesRead) {
        switch (ri->currentState) {

            case SEARCHING_START_BYTE:
                for (; ri->currentInputIndex < bytesRead; ri->currentInputIndex++) {
                    if (pBuff[ri->currentInputIndex] == FT12_START_BYTE) {
                        destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                        ri->currentState = CHECKING_FIRST_LENGTH;
                        break;
                    }
                }
            break;

            case CHECKING_FIRST_LENGTH:
                destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                ri->currentState = CHECKING_SECOND_LENGTH;

            default:
                break;
        }
    }

    return currentState;

    /*int startByteIndex = -1;
    int endByteStartIndex = -1;
    int patternCheckingStartIndex = 0;

    while (patternCheckingStartIndex < buffLen) {
        startByteIndex = lookForFT12Start(pBuff, buffLen, &patternCheckingStartIndex);

        if(startByteIndex < 0) {
            return currentState;
        }

        endByteStartIndex = checkPattern(pBuff, startByteIndex);

        if(endByteStartIndex >= 0)
            break;

        patternCheckingStartIndex++;
    }

    if(endByteStartIndex < 0) {
        return currentState;
    }

    if(!checkChecksum(pBuff, startByteIndex, endByteStartIndex)) {
        currentState = CHECKSUM_ERROR;
        return currentState;
    }
    currentState = FRAME_OK;
    memcpy_s(destBuff,
        FT12_ARRAY_LENGTH,
        pBuff + startByteIndex,
        GET_FT12_LENGTH(startByteIndex, endByteStartIndex));

    return currentState;*/
}

// Thread function to handle user input.
DWORD WINAPI InputThread(LPVOID lpParameter) {
    getchar();

    return 0;
}

int main(int argc, char const *argv[]) {
    // Test input buffer array
    /*const unsigned char testPack[INPUT_ARRAY_LENGTH] =   {0x68, 0x68, 0x16, 0x68,
                                                            0x68, 0x68, 0x0c, 0x0c,
                                                            0x68, 0x73, 0xf0, 0x06,
                                                            0x00, 0x03, 0x00, 0x01,
                                                            0x00, 0x03, 0x03, 0x01,
                                                            0x01, 0x75, 0x16, 0x0e};*/

    /*while(true) {
        // check for data on port and display it on screen.
        ReadFile(serialHandle, buffer, sizeof(buffer), &dwRead, NULL);
        if(dwRead) {
            printf("%d\n", readBuffer(buffer, sizeof(buffer), ft12Buffer));

            printBuff(ft12Buffer, FT12_ARRAY_LENGTH);

            /*printBuff(buffer, sizeof(buffer));#1#
        }
    }*/

    // Open the serial port in overlapped mode.
    // Open the serial port in overlapped mode.
    HANDLE hSerial = CreateFile(
        "COM3",                        // Change this as needed (e.g., "COM3")
        GENERIC_READ | GENERIC_WRITE,
        0,                             // No sharing
        NULL,                          // Default security attributes
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,          // Overlapped mode
        NULL
    );
    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening serial port\n");
        return 1;
    }

    // Configure serial port parameters.
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error getting serial port state\n");
        CloseHandle(hSerial);
        return 1;
    }
    dcbSerialParams.BaudRate = CBR_19200;
    dcbSerialParams.ByteSize   = 8;
    dcbSerialParams.StopBits   = ONESTOPBIT;
    dcbSerialParams.Parity     = EVENPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "Error setting serial port state\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Optionally, set timeouts.
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout         = 50;
    timeouts.ReadTotalTimeoutConstant    = 50;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.WriteTotalTimeoutConstant   = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Create a thread to handle user input.
    DWORD threadId;
    HANDLE hThread = CreateThread(
        NULL, 0, InputThread, hSerial, 0, &threadId
    );
    if (hThread == NULL) {
        fprintf(stderr, "Error creating input thread\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Main thread: asynchronous read loop.
    char buffer[INPUT_ARRAY_LENGTH] = {0};
    char ft12Frame[FT12_ARRAY_LENGTH] = {0};
    DWORD bytesRead = 0;
    OVERLAPPED osRead = {0};
    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osRead.hEvent == NULL) {
        fprintf(stderr, "Error creating event for read\n");
        CloseHandle(hSerial);
        return 1;
    }

    ReaderInfo ri{
    SEARCHING_START_BYTE,
    0,
    0,
    0};

    printf("Press [Enter] to stop listening...\n");

    // Loop: perform asynchronous reads.
    // (This loop will run until the input thread ends, e.g., when EOF is received on stdin.)
    while (true) {
        // Initiate an asynchronous read.
        BOOL readResult = ReadFile(
            hSerial,
            buffer,
            sizeof(buffer),
            &bytesRead,
            &osRead
        );
        if (!readResult) {
            if (GetLastError() == ERROR_IO_PENDING) {
                // Wait briefly for the read to complete.
                DWORD waitRes = WaitForSingleObject(osRead.hEvent, 100);
                if (waitRes == WAIT_TIMEOUT) {
                    // Check if the input thread is still active.
                    if (WaitForSingleObject(hThread, 0) == WAIT_OBJECT_0) {
                        // Input thread ended (e.g., user closed stdin); exit the loop.
                        break;
                    }
                    continue;
                }
                if (!GetOverlappedResult(hSerial, &osRead, &bytesRead, FALSE)) {
                    fprintf(stderr, "Error in GetOverlappedResult for read\n");
                    break;
                }
            } else {
                fprintf(stderr, "Error in ReadFile\n");
                break;
            }
        }

        if (bytesRead > 0) {
            readBuffer(buffer, bytesRead, ft12Frame, &ri);
            printBuff(ft12Frame, sizeof(ft12Frame));
        }
        // Reset the event for the next read.
        ResetEvent(osRead.hEvent);
    }

    // Clean up: wait for the input thread to finish and close all handles.
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(osRead.hEvent);
    CloseHandle(hSerial);
    return EXIT_SUCCESS;
}
