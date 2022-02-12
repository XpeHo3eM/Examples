#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>



enum class Errors
{
    NONE,
    WSA_STARTUP,
    GET_ADDR_INFO,
    SOCKET_CREATION,
    BINDING_SOCKET
};



struct SocketData
{
    WSADATA   wsaData;
    ADDRINFO  hints;
    ADDRINFO *addrResult   = nullptr;
    SOCKET    clientSocket = INVALID_SOCKET;
    SOCKET    listenSocket = INVALID_SOCKET;
};



class Server
{
public:
    Errors Start(const char *port);

private:
    SocketData socketData;
    static const size_t BUFSIZE = 512;
    
    void receiveData(SOCKET &socket);
    void shutdownSocket(SOCKET &socket);
    int charToInt(char *buf);
};