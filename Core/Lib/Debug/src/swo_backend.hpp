#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <cstdint>

#define ITM_STIM_U32 (*(volatile unsigned int *)0xE0000000) // Stimulus Port Register word acces
#define ITM_STIM_U8 (*(volatile char *)0xE0000000)          // Stimulus Port Register byte acces
#define ITM_ENA (*(volatile unsigned int *)0xE0000E00)      // Trace Enable Ports Register
#define ITM_TCR (*(volatile unsigned int *)0xE0000E80)      // Trace control register

void printChar(int port, char c);
void printCharArray(int port, const char *s);
void printString(int port, std::string &s);