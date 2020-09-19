#include "cuteprint.h"

std::ofstream logger; 
std::ofstream debugger; 

void logger_init(const std::string &name)
{ 
    if (logger.is_open())
        logger.close(); 
    logger.open(name); 
}

void debuger_init(const std::string &name)
{ 
    if (debugger.is_open())
        debugger.close(); 
    debugger.open(name); 
}

void printer_destroy()
{
    if (logger.is_open())
        logger.close();
    if(debugger.is_open())
        debugger.close(); 
}