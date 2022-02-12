#include <iostream>

#include "server.h"



int main()
{
    Server myServ;
    const char *port = "80085";
    
    myServ.Start(port);
}