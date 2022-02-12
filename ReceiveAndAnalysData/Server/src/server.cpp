#pragma comment (lib, "ws2_32.lib")

#include <iostream>

#include "server.h"



Errors Server::Start(const char *port)
{
    int err;

    err = WSAStartup(MAKEWORD(2, 2), &socketData.wsaData);
    if (err)
    {
        std::cout << "WSAStartup error code " << err << std::endl;

        return Errors::WSA_STARTUP;
    }

    ZeroMemory(&socketData.hints, sizeof(socketData.hints));
    socketData.hints.ai_family   = AF_INET;
    socketData.hints.ai_socktype = SOCK_STREAM;
    socketData.hints.ai_protocol = IPPROTO_TCP;
    socketData.hints.ai_flags    = AI_PASSIVE;

    err = getaddrinfo(nullptr, port, &socketData.hints, &socketData.addrResult);
    if (err)
    {
        std::cout << "Getaddrinfo error code " << err << std::endl;

        WSACleanup();
        return Errors::GET_ADDR_INFO;
    }

    socketData.listenSocket = socket(socketData.addrResult->ai_family,
                                     socketData.addrResult->ai_socktype,
                                     socketData.addrResult->ai_protocol);
    if (socketData.listenSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation error" << std::endl;

        freeaddrinfo(socketData.addrResult);
        WSACleanup();
        return Errors::SOCKET_CREATION;
    }
        
    err = bind(socketData.listenSocket,
               socketData.addrResult->ai_addr,
               socketData.addrResult->ai_addrlen);
    if (err == SOCKET_ERROR)
    {
        std::cout << "Binding socket failed" << std::endl;

        shutdownSocket(socketData.listenSocket);
        
        return Errors::BINDING_SOCKET;
    }

    while (true)
    {
        while (socketData.clientSocket == INVALID_SOCKET)
        {
            listen(socketData.listenSocket, SOMAXCONN);
            socketData.clientSocket = accept(socketData.listenSocket, nullptr, nullptr);
        }

        receiveData(socketData.clientSocket);
    }
}



void Server::receiveData(SOCKET &socket)
{
    char buf[BUFSIZE];
    bool exit = false;
    while (!exit)
    {
        ZeroMemory(buf, BUFSIZE);
        auto err = recv(socket, buf, BUFSIZE, 0);
        if (err > 0)
        {
            int data = charToInt(buf);
            if ((data / 100 >= 1) && (data % 32 == 0))
            {
                std::cout << "~ Data receive ~" << std::endl;
            }
            else
            {
                std::cout << "! Incorrect data !" << std::endl;
            }
        }
        else
        {
            shutdownSocket(socket);
            exit = true;
        }
    }
}



void Server::shutdownSocket(SOCKET &socket)
{
    closesocket(socket);
    socket = INVALID_SOCKET;
}



int Server::charToInt(char *buf)
{
    int res = 0;
    size_t len = strlen(buf);
    for (int i = 0; i < len; ++i)
    {
        res *= 10;
        res += buf[i] - '0';
    }

    return res;
}