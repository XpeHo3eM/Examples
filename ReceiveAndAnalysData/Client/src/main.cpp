#include <iostream>
#include <thread>

#include "Client.h"



int main()
{
    Client client("localhost", "80085");
    
    while (client.Connect() != Errors::NONE);

    std::thread t1([&]() { client.PartOne(); });
    std::thread t2([&]() { client.PartTwo(); });

    t1.join();
    t2.join();
}