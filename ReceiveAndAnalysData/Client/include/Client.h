#pragma once

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <mutex>
#include <string>
#include <string_view>

#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>



enum class Errors
{
    NONE,
    WSA_STARTUP,
    GET_ADDR_INFO,
    SOCKET_CREATION,
    CONNECT_CREATION
};



struct SocketData
{
    WSADATA  wsaData;
    ADDRINFO hints;
    ADDRINFO *addrResult   = nullptr;
    SOCKET   connectSocket = INVALID_SOCKET;
};



class Client
{
public:
    Client(const char *hostname, const char *port);
    void    PartOne();
    void    PartTwo();
    Errors  Connect();

private:
    SocketData socketData;
    std::mutex mPrint;
    std::mutex mRec;
    std::mutex mSend;
    std::condition_variable cvRec;
    std::condition_variable cvSend;
    const std::string tmpName = "buffer.txt";
    const int strMaxSize      = 64;
    const char *hostname      = nullptr;
    const char *port          = nullptr;

    std::string TakeData();
    void SortData(std::string &str);
    void TransformData(std::string &str);
    void SaveData(std::string_view str);
    std::string LoadData();
    int SummaryData(std::string_view str);
    void ShutdownConnection();
};