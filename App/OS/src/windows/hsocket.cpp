#include "hsocket.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <sstream>
#include "hcore.hpp"
#include "OScommon.hpp"


#pragma comment(lib, "Ws2_32.lib")


namespace OS{

    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ConnectSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    extern bool socket_connected = false;

    SocketMode socketMode;


    void socket_set_core(HCore::HCore *core){
        
    }


    void socket_init(SocketMode sm){
        socketMode = sm;
        iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
        if (iResult != 0) {
            m_core->log(LOG_ERROR,"WSAStartup failed: %d", iResult);
        }
    }

    int socket_connect(const char *server, uint16_t port){
        std::ostringstream portString;
        portString << port;
        
        struct addrinfo *result = NULL,
                        *ptr = NULL,
                        hints;

        ZeroMemory( &hints, sizeof(hints) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        iResult = getaddrinfo(server, portString.str().c_str(), &hints, &result);
        if (iResult != 0) {
            m_core->log(LOG_ERROR,"Unable to connect to server!: %d", iResult);
            WSACleanup();
            return -1;
        }

        ptr=result;

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
            m_core->log(LOG_ERROR,"Error at socket(): %ld", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return -1;
        }

        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
        }

        freeaddrinfo(result);

        if (ConnectSocket == INVALID_SOCKET) {
            m_core->log(LOG_ERROR,"Unable to connect to server!");
            WSACleanup();
            return -1;
        }

        return 0;
    }

    int socket_listen(uint16_t port){
        std::ostringstream portString;
        portString << port;

        struct addrinfo *result = NULL, *ptr = NULL, hints;

        ZeroMemory(&hints, sizeof (hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the local address and port to be used by the server
        iResult = getaddrinfo(NULL, portString.str().c_str(), &hints, &result);
        if (iResult != 0) {
            m_core->log(LOG_ERROR,"Unable to connect to server!: %d", iResult);
            WSACleanup();
            return -1;
        }
        
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

        if (ListenSocket == INVALID_SOCKET) {
            m_core->log(LOG_ERROR,"Error at socket(): %ld", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return -1;
        }

        iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            m_core->log(LOG_ERROR,"bind failed with error: %d", WSAGetLastError());
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();
            return -1;
        }

        freeaddrinfo(result);

        if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
            m_core->log(LOG_ERROR,"Listen failed with error: %ld", WSAGetLastError() );
            closesocket(ListenSocket);
            WSACleanup();
            return -1;
        }

        return 0;
    }

    int socket_accept(){
        ConnectSocket = INVALID_SOCKET;
        // Accept a client socket
        ConnectSocket = accept(ListenSocket, NULL, NULL);
        if (ConnectSocket == INVALID_SOCKET) {
            m_core->log(LOG_ERROR,"accept failed: %d", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return -1;
        }
    

        return 0;
    }

    int socket_receive(void *data, unsigned int size){
        char *dst = (char*)data;
        int n;
        while(size) {
            n = recv(ConnectSocket, dst, size,0);
            if (n == SOCKET_ERROR) {
                m_core->log(LOG_ERROR,"Unable to connect to server!: %d", WSAGetLastError());
                return 1;
            }
            size -= n;
            dst += n;
        }

        return size;
    }

    int socket_send(void *data, unsigned int size){
        char *dst = (char*)data;
        int n;
        
        while(size) {
            n = send(ConnectSocket, dst, size,0);
            if (n == SOCKET_ERROR) {
                m_core->log(LOG_ERROR,"Unable to connect to server!: %d", WSAGetLastError());
                return 1;
            }
            size -= n;
            dst += n;
        }

        return size;
    }

    void socket_close(){
        closesocket(ConnectSocket);
    }

}