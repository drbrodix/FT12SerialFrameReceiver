#include "FT12SerialFrameReceiver.h"

void printBuff(const unsigned char* pBuff, DWORD buffLen) {
    for (DWORD i = 0; i < buffLen; i++)
    {
        printf("%x ", pBuff[i]);
    }
    printf("\n");
}

STATES readBuffer(const unsigned char* pBuff, const DWORD bytesRead, unsigned char* destBuff, ReaderInfo* ri) {

    // typedef enum {
    //     SEARCHING_START_BYTE        = 0,
    //     CHECKING_FIRST_LENGTH       = 1,
    //     CHECKING_SECOND_LENGTH      = 2,
    //     CHECKING_SECOND_START_BYTE  = 3,
    //     CHECKING_CONTROL_BYTE       = 4,
    //     CHECKING_BAOS_PAYLOAD       = 5,
    //     CHECKING_CHECKSUM           = 6,
    //     CHECKING_END_BYTE           = 7,
    // } STATES;
    //
    // typedef struct {
    //     STATES currentState;
    //     DWORD currentInputIndex;
    //     DWORD currentOutputIndex;
    //     unsigned char payloadLength;
    //     int checksumSum;
    //     bool doStartBytesMatch;
    //     bool doLengthBytesMatch;
    //     bool doesChecksumMatch;
    //     bool isEndByteFound;
    // } ReaderInfo;

    while (ri->currentInputIndex < bytesRead) {
        switch (ri->currentState) {

            case SEARCHING_START_BYTE:
                while (ri->currentInputIndex < bytesRead) {
                    if (pBuff[ri->currentInputIndex] == FT12_START_BYTE) {
                        destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                        ri->currentState = CHECKING_FIRST_LENGTH;
                        ri->currentInputIndex++;
                        ri->currentOutputIndex++;
                        break;
                    }
                    ri->currentInputIndex++;
                    ri->currentOutputIndex++;
                }
            break;

            case CHECKING_FIRST_LENGTH:
                destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                ri->payloadLength = pBuff[ri->currentInputIndex];
                ri->currentState = CHECKING_SECOND_LENGTH;
                ri->currentInputIndex++;
                ri->currentOutputIndex++;
            break;

            case CHECKING_SECOND_LENGTH:
                destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                ri->doLengthBytesMatch = destBuff[ri->currentOutputIndex] == destBuff[ri->currentOutputIndex - 1];
                ri->currentState = CHECKING_SECOND_START_BYTE;
                ri->currentInputIndex++;
                ri->currentOutputIndex++;
            break;

            case CHECKING_SECOND_START_BYTE:
                destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                ri->doStartBytesMatch = destBuff[ri->currentOutputIndex] == destBuff[ri->currentOutputIndex - 3];
                ri->currentState = CHECKING_CONTROL_BYTE;
                ri->currentInputIndex++;
                ri->currentOutputIndex++;
            break;

            case CHECKING_CONTROL_BYTE:
                destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                ri->checksumSum = destBuff[ri->currentOutputIndex];
                ri->currentState = CHECKING_BAOS_PAYLOAD;
                ri->currentInputIndex++;
                ri->currentOutputIndex++;
            break;

            case CHECKING_BAOS_PAYLOAD:
                // It must be checked, that we don't try to access empty elements in the array,
                // and it is also necessary to ensure that we only access the payload part of the frame.
                while  ((ri->currentInputIndex < bytesRead) &&
                        (ri->currentOutputIndex <= GET_PAYLOAD_END_INDEX(ri->payloadLength))) {

                    destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                    ri->checksumSum += destBuff[ri->currentOutputIndex];
                    ri->currentInputIndex++;
                    ri->currentOutputIndex++;
                }
                ri->currentState = CHECKING_CHECKSUM;
            break;

            case CHECKING_CHECKSUM:
                destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                ri->doesChecksumMatch = (ri->checksumSum % 256) == destBuff[ri->currentOutputIndex];
                ri->currentState = CHECKING_END_BYTE;
                ri->currentInputIndex++;
                ri->currentOutputIndex++;
            break;

            case CHECKING_END_BYTE:
                destBuff[ri->currentOutputIndex] = pBuff[ri->currentInputIndex];
                ri->isEndByteFound = (destBuff[ri->currentOutputIndex] == FT12_END_BYTE);
                ri->currentState = RECEPTION_COMPLETE;
                ri->currentInputIndex++;
                ri->currentOutputIndex++;
                return ri->currentState;
            break;

            default:
                break;
        }
    }

    return ri->currentState;
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

    ReaderInfo ri = {0};

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

        // If actual data has been received
        if (bytesRead > 0) {
            // While the buffer has not been completely read
            while (ri.currentInputIndex < bytesRead) {
                readBuffer(buffer, bytesRead, ft12Frame, &ri);

                printBuff(ft12Frame, sizeof(ft12Frame));
                printf("Start byte match: %d\n"
                       "Length byte match: %d\n"
                       "Checksum match: %d\n"
                       "End byte found: %d\n"
                       "Data left in input buffer... Data: %d\n",
                                ri.doStartBytesMatch,
                                ri.doLengthBytesMatch,
                                ri.doesChecksumMatch,
                                ri.isEndByteFound,
                                bytesRead - ri.currentInputIndex);

                // If the frame has been completely read
                if (ri.currentState == RECEPTION_COMPLETE) {
                    ri.currentState = SEARCHING_START_BYTE;
                    const DWORD currentInputIndex = ri.currentInputIndex;
                    memset(&ft12Frame, 0, sizeof(ft12Frame));
                    memset(&ri, 0, sizeof(ri));
                    ri.currentInputIndex = currentInputIndex;
                }
            }
            // Reset buffer and reader struct if buffer has been completely read
                memset(&ri, 0, sizeof(ri));
                memset(&ft12Frame, 0, sizeof(ft12Frame));
                memset(&buffer, 0, sizeof(buffer));
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
