#include "FT12SerialFrameReceiver.h"

void printBuff(const unsigned char *pBuff, const DWORD startIndex, const DWORD endIndex)
{
    for (DWORD i = startIndex; i <= endIndex; i++)
    {
        printf("%x ", pBuff[i]);
    }
    printf("\n");
}

STATES stateMachine(unsigned char *pBuff, const DWORD bytesRead, ReaderInfo *ri)
{
    // // Enumerator of the used states of the state machine
    // typedef enum {
    //     SEARCHING_START_BYTE        = 0,
    //     CHECKING_FIRST_LENGTH       = 1,
    //     CHECKING_SECOND_LENGTH      = 2,
    //     CHECKING_SECOND_START_BYTE  = 3,
    //     HEADER_FOUND                = 4
    // } STATES;
    //
    // // Structure to keep track of necessary infos
    // // related to the buffer analysis process
    // typedef struct {
    //     STATES currentState;
    //     DWORD currentIndex;
    //     DWORD currentRoundIndex;
    //     DWORD startByteIndex;
    //     DWORD endByteIndex;
    //     unsigned char payloadLength;
    //     bool resetRI;
    // } ReaderInfo;
    while (ri->currentIndex < bytesRead)
    {
        switch (ri->currentState)
        {
            // Base state, looking for start byte of a frame
            case SEARCHING_START_BYTE:
                // Start byte 0x68 found
                if (pBuff[ri->currentIndex] == FT12_START_BYTE)
                {
                    // Mark index as startByteIndex
                    ri->startByteIndex = ri->currentIndex;

                    // Move 1 byte forward for next byte read
                    ri->currentIndex++;

                    // Set state for next step
                    ri->currentState = CHECKING_FIRST_LENGTH;

                    break;
                }
                // Start byte not found, try the next byte
                else
                {
                    // Move 1 byte forward for next byte read
                    ri->currentIndex++;
                }
                break;

            case CHECKING_FIRST_LENGTH:
                // Assuming it's the payload length,
                // store it in a variable for further use
                ri->payloadLength = pBuff[ri->currentIndex];

                // Move 1 byte forward for next byte read
                ri->currentIndex++;

                // Set state for next step
                ri->currentState = CHECKING_SECOND_LENGTH;

                break;

            case CHECKING_SECOND_LENGTH:
                // Check whether the supposed second
                // length byte matches with the first one
                if (pBuff[ri->currentIndex] == pBuff[ri->currentIndex - 1])
                {
                    // Set state for next step if length bytes match
                    ri->currentState = CHECKING_SECOND_START_BYTE;
                }
                // Length bytes don't match, hence not a valid FT 1.2 header.
                // Back to looking for start byte
                else
                {
                    // Set state for going back to
                    // looking for start byte
                    ri->currentState = SEARCHING_START_BYTE;
                }
                // Move 1 byte forward for next byte read
                ri->currentIndex++;
                break;

            case CHECKING_SECOND_START_BYTE:
                // Check whether both start bytes are found
                // at expected offset from each other
                if (pBuff[ri->currentIndex] == pBuff[ri->currentIndex - 3])
                {
                    // If both start bytes are found,
                    // set the state accordingly
                    ri->currentState = HEADER_FOUND;
                }
                // If start bytes don't match, mark the
                // current index as garbageIndex, since
                // the previous bytes are not part of a frame
                else
                {
                    ri->currentState = SEARCHING_START_BYTE;
                }
                ri->currentIndex++;
                break;

            case HEADER_FOUND:
                // Check if there's garbage before the actual frame to Wireshark
                if (pBuff[ri->currentRoundIndex] != pBuff[ri->startByteIndex])
                {
                    // Push the garbage before the actual frame to Wireshark
                    printBuff(pBuff, ri->currentRoundIndex, ri->startByteIndex - 1);
                }

                ri->endByteIndex = FIND_FT12_END_BYTE(ri->startByteIndex, ri->payloadLength);

                // If rest of frame is in buffer
                if (ri->endByteIndex < bytesRead)
                {
                    // Push the frame to Wireshark
                    printBuff(pBuff, ri->startByteIndex, ri->endByteIndex);

                    // Set reader index one byte after the
                    // end byte of the last pushed frame
                    ri->currentIndex = ri->endByteIndex + 1;

                    // Mark the start index of the next round of reading.
                    // Used to calculate the range of garbage.
                    ri->currentRoundIndex = ri->currentIndex;

                    // Set state back to start byte searching
                    ri->currentState = SEARCHING_START_BYTE;
                }
                // Only a segment of the frame is in the current buffer
                else
                {
                    // Move partial frame to the start of the buffer
                    memmove(pBuff, pBuff + ri->startByteIndex, bytesRead - ri->startByteIndex);
                    // Adjust indices
                    ri->currentIndex = bytesRead - ri->startByteIndex + 1;
                    ri->startByteIndex, ri->currentRoundIndex = 0;
                    fprintf(stderr, "Rest of frame is not in buff!\n");
                }
                break;

            default:
                break;
        }
    }

    // Reached if buffer has been completely read
    return ri->currentState;
}

HANDLE createHandle(LPCSTR fileName)
{
    HANDLE hSerial = CreateFile
                                (
                                    fileName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_OVERLAPPED,
                                    NULL
                                );
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Error opening serial port\n");
    }
    return hSerial;
}

WINBOOL configPort(HANDLE hSerial)
{
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        fprintf(stderr, "Error getting serial port state\n");
        CloseHandle(hSerial);
        return false;
    }
    dcbSerialParams.BaudRate    = CBR_19200;
    dcbSerialParams.ByteSize    = 8;
    dcbSerialParams.StopBits    = ONESTOPBIT;
    dcbSerialParams.Parity      = EVENPARITY;
    if (!SetCommState(hSerial, &dcbSerialParams))
    {
        fprintf(stderr, "Error setting serial port state\n");
        CloseHandle(hSerial);
        return false;
    }
    return true;
}

WINBOOL configTimeouts(HANDLE hSerial)
{
    COMMTIMEOUTS timeouts                   = {0};
    timeouts.ReadIntervalTimeout            = 50;
    timeouts.ReadTotalTimeoutConstant       = 50;
    timeouts.ReadTotalTimeoutMultiplier     = 10;
    timeouts.WriteTotalTimeoutConstant      = 50;
    timeouts.WriteTotalTimeoutMultiplier    = 10;
    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        fprintf(stderr, "Error setting timeouts\n");
        CloseHandle(hSerial);
        return false;
    }
    return true;
}

DWORD WINAPI InputThread(LPVOID lpParameter)
{
    getchar();
    return 0;
}

int main(int argc, char const *argv[])
{
    ////                                                            ////
    //// COMMENTED SECTION BELOW HAS BEEN KEPT FOR TESTING PURPOSES ////
    ////                                                            ////

    /*ReaderInfo ri = {0};
    unsigned char ft12Frame[FT12_ARRAY_LENGTH] = {0};

    // Test input buffer arrays
    DWORD bytesRead1 = 12;
    const unsigned char testPack1[INPUT_ARRAY_LENGTH] = {0x00, 0x68, 0x0c, 0x0c,
                                                         0x68, 0x73, 0xf0, 0x06,
                                                         0x00, 0x03, 0x00, 0x01};

    DWORD bytesRead2 = 7;
    const unsigned char testPack2[INPUT_ARRAY_LENGTH] = {0x00, 0x03, 0x03, 0x01,
                                                         0x01, 0x75, 0x16};

    while (ri.currentInputIndex < bytesRead1) {
        readFrame(testPack1, bytesRead1, ft12Frame, &ri);

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
                        bytesRead1 - ri.currentInputIndex);

        // If the frame has been completely read
        if (ri.currentState == RECEPTION_COMPLETE) {
            const DWORD currentInputIndex = ri.currentInputIndex;
            memset(&ft12Frame, 0, sizeof(ft12Frame));
            memset(&ri, 0, sizeof(ri));
            ri.currentInputIndex = currentInputIndex;
        }
    }

    // Reset buffer and reader struct if buffer has been completely read
    memset(testPack1, 0, sizeof(testPack1));
    ri.currentInputIndex = 0;

    while (ri.currentInputIndex < bytesRead2) {
        readFrame(testPack2, bytesRead2, ft12Frame, &ri);

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
                        bytesRead2 - ri.currentInputIndex);

        // If the frame has been completely read
        if (ri.currentState == RECEPTION_COMPLETE) {
            const DWORD currentInputIndex = ri.currentInputIndex;
            memset(&ft12Frame, 0, sizeof(ft12Frame));
            memset(&ri, 0, sizeof(ri));
            ri.currentInputIndex = currentInputIndex;
        }
    }*/


    /*const unsigned char testPack3[INPUT_ARRAY_LENGTH] =  {0x00, 0x68, 0x0c, 0x0c,
                                                            0x68, 0x73, 0xf0, 0x06,
                                                            0x00, 0x03, 0x00, 0x01,
                                                            0x00, 0x03, 0x03, 0x01,
                                                            0x01, 0x75, 0x16, 0x0e};*/

    // Open the serial port in overlapped mode
    HANDLE hSerial = createHandle("\\\\.\\COM3");
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        return EXIT_FAILURE;
    }

    // Configure serial port parameters
    if (!configPort(hSerial))
    {
        return EXIT_FAILURE;
    }

    // Set timeouts
    if (!configTimeouts(hSerial))
    {
        return EXIT_FAILURE;
    }

    // Create a thread to handle user input
    DWORD threadId;
    HANDLE hThread = CreateThread(
        NULL, 0, InputThread, hSerial, 0, &threadId
    );
    if (hThread == NULL)
    {
        fprintf(stderr, "Error creating input thread\n");
        CloseHandle(hSerial);
        return 1;
    }

    // Main thread: asynchronous read loop
    char buffer[INPUT_ARRAY_LENGTH] = {0};
    DWORD bytesRead = 0;
    OVERLAPPED osRead = {0};
    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osRead.hEvent == NULL)
    {
        fprintf(stderr, "Error creating event for read\n");
        CloseHandle(hSerial);
        return EXIT_FAILURE;
    }

    ReaderInfo ri = {0};

    printf("Press [Enter] to stop listening...\n");

    // This loop will run until the input thread ends
    while (true)
    {
        // Initiate an asynchronous read
        BOOL readResult = ReadFile(
            hSerial,
            buffer + ri.currentIndex,
            sizeof(buffer),
            &bytesRead,
            &osRead
        );
        if (!readResult)
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                // Wait briefly for the read to complete
                DWORD waitRes = WaitForSingleObject(osRead.hEvent, 100);
                if (waitRes == WAIT_TIMEOUT)
                {
                    // Check if the input thread is still active.
                    if (WaitForSingleObject(hThread, 0) == WAIT_OBJECT_0)
                    {
                        // Input thread ended, terminate the loop
                        break;
                    }
                    continue;
                }
                if (!GetOverlappedResult(hSerial, &osRead, &bytesRead, FALSE))
                {
                    fprintf(stderr, "Error in GetOverlappedResult for read\n");
                    break;
                }
            } else
            {
                fprintf(stderr, "Error in ReadFile\n");
                break;
            }
        }

        // If actual data has been received
        if (bytesRead > 0)
        {
            // The state machine should return with HEADER_FOUND state
            // only if a partial frame has been read. In this case
            // keeping the ReaderInfos is necessary, hence the structure
            // should not be reset. The structure needs to be reset otherwise.
            if (stateMachine(buffer, bytesRead, &ri) != HEADER_FOUND)
            {
                ZeroMemory(&ri, sizeof(ReaderInfo));
            }
        }
        // Reset the event for the next read.
        ResetEvent(osRead.hEvent);
    }
    CloseHandle(hThread);
    CloseHandle(osRead.hEvent);
    CloseHandle(hSerial);
    return EXIT_SUCCESS;
}
