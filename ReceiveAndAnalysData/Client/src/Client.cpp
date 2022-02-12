#pragma comment (lib, "ws2_32.lib")

#include <iostream>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <chrono>
#include <string>
#include <string_view>

#include "Client.h"



Client::Client(const char *inHostname, const char *inPort)
    : hostname(inHostname), port(inPort)
{}



Errors Client::Connect()
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

    err = getaddrinfo(hostname, port, &socketData.hints, &socketData.addrResult);
    if (err)
    {
        std::cout << "Getaddrinfo error code " << err << std::endl;

        WSACleanup();
        return Errors::GET_ADDR_INFO;
    }

    socketData.connectSocket = socket(socketData.addrResult->ai_family,
                                      socketData.addrResult->ai_socktype,
                                      socketData.addrResult->ai_protocol);
    if (socketData.connectSocket == INVALID_SOCKET)
    {
        std::cout << "Socket creation error" << std::endl;

        freeaddrinfo(socketData.addrResult);
        WSACleanup();
        return Errors::SOCKET_CREATION;
    }

    err = connect(socketData.connectSocket, 
                  socketData.addrResult->ai_addr, 
                  socketData.addrResult->ai_addrlen);
    if (err == SOCKET_ERROR)
    {
        std::cout << "Unable connet to server" << std::endl;

        ShutdownConnection();
        return Errors::CONNECT_CREATION;
    }

    std::cout << "Connected to server" << std::endl;
    return Errors::NONE;
}



void Client::PartOne()
{
    std::unique_lock<std::mutex> ulRec(mRec);
    ulRec.unlock();

    std::string str = "";
    while (true)
    {
        std::unique_lock<std::mutex> ulPrint(mPrint);
        std::cout << "Enter a string with numbers only (size max is " << strMaxSize << "): ";
        str = TakeData();
        ulPrint.unlock();

        SortData(str);
        TransformData(str);
        SaveData(str);
    }
}



void Client::PartTwo()
{
    std::string str = "";
    while (true)
    {
        str = LoadData();
        auto result = SummaryData(str);
        auto data = std::to_string(result);
        int err = send(socketData.connectSocket, 
                       &data[0],
                       data.size(), 
                       0);

        if (err == SOCKET_ERROR)
        {
            std::unique_lock<std::mutex> ulPrint(mPrint);
            std::cout << "Unable send data ..." << std::endl;
            ShutdownConnection();
            std::cout << "Trying to reconnect to server ..." << std::endl;
            while(Connect() != Errors::NONE);
            ulPrint.unlock();
        }
    }
}



std::string Client::TakeData()
{
    std::string str = "";
    bool isCorrectStr = false;
    while (!isCorrectStr)
    {
        isCorrectStr = true;
        std::cin >> str;

        int size = str.size();
        isCorrectStr = (size > 0 && size <= strMaxSize);

        for (int i = 0; (i < size) && isCorrectStr; ++i)
        {
            isCorrectStr = isdigit(str[i]);
        }
    }

    return str;
}



void Client::SortData(std::string &str)
{
    std::sort(str.begin(), str.end(), [](char a, char b) { return (a > b); });
}



void Client::TransformData(std::string &str)
{
    std::string tmp = "";
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] % 2 == 0)
        {
            tmp += "KB";
        }
        else
        {
            tmp += str[i];
        }
    }
    str = std::move(tmp);
}



void Client::SaveData(std::string_view str)
{
    std::unique_lock<std::mutex> ulRec(mRec);
    std::ofstream file(tmpName);
    file << str;
    file.close();
    ulRec.unlock();

    cvRec.notify_one();
}



std::string Client::LoadData()
{
    std::unique_lock<std::mutex> ulRec(mRec);
    std::string str = "";
    bool isEmpty = true;
    do
    {
        cvRec.wait(ulRec);

        std::ifstream file(tmpName, std::ios::binary);
        file.seekg(0, file.end);
        isEmpty = file.tellg() == 0;
        file.close();
    }
    while (isEmpty);
    
    std::ifstream file(tmpName);

    if (file)
    {
        file >> str;
        file.close();
        remove(&tmpName[0]);
    }
    else
    {
        std::unique_lock<std::mutex> ulPrint(mPrint);
        std::cout << "Tmp file not found";
    }

    return str;
}



int Client::SummaryData(std::string_view str)
{
    int sum = 0;
    for (int i = 0; i < str.size(); ++i)
    {
        if (isdigit(str[i]))
            sum += str[i] - '0';
        else
            ++i;
    }

    return sum;
}



void Client::ShutdownConnection()
{
    closesocket(socketData.connectSocket);
    freeaddrinfo(socketData.addrResult);
    socketData.connectSocket = INVALID_SOCKET;
    WSACleanup();
}