/* logger */
#pragma once

#include <fstream> 
#include <string> 

extern std::ofstream logger; 
extern std::ofstream debugger; 

void logger_init(const std::string &name);
void debuger_init(const std::string &name);
void printer_destroy();
