#include "swo_backend.hpp"

void printChar(int port, char c)
{
    //
    // Check if ITM_TCR.ITMENA is set
    //
    if ((ITM_TCR & 1) == 0)
    {
        return;
    }
    //
    // Check if stimulus port is enabled
    //
    if ((ITM_ENA & (1 << port)) == 0)
    {
        return;
    }
    //
    // Wait until STIMx is ready,
    // then send data
    //
    volatile char *ITM_STIM = (volatile char *)(0xE0000000 + port * 4);
    while ((*ITM_STIM & 1) == 0)
        ;
    *ITM_STIM = c;
}

void printCharArray(int port, const char *s)
{
    while (*s)
    {
        printChar(port, *s++);
    }
}

void printString(int port, std::string &s)
{
    printCharArray(port, s.c_str());
}