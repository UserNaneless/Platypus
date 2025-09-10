
#include "buffer.hpp"
#include "debug_terminal.hpp"
#include "swo_backend.hpp"

int SWOStreamBuffer::overflow(int c) {
    if (c != EOF) {
        std::string s(1, static_cast<char>(c));
        printString(port, s);
    }
    return c;
}

SWOStreamBuffer customBuf0(0);
SWOStreamBuffer customBuf1(1);
SWOStreamBuffer customBuf2(2);
SWOStreamBuffer customBuf3(3);
SWOStreamBuffer customBuf4(4);
SWOStreamBuffer customBuf5(5);
SWOStreamBuffer customBuf6(6);
SWOStreamBuffer customBuf7(7);

SWOStreamBuffer customBuf8(8);
SWOStreamBuffer customBuf9(9);
SWOStreamBuffer customBuf10(10);
SWOStreamBuffer customBuf11(11);
SWOStreamBuffer customBuf12(12);
SWOStreamBuffer customBuf13(13);
SWOStreamBuffer customBuf14(14);
SWOStreamBuffer customBuf15(15);

std::ostream swoTerminal0(&customBuf0);
std::ostream swoTerminal1(&customBuf1);
std::ostream swoTerminal2(&customBuf2);
std::ostream swoTerminal3(&customBuf3);
std::ostream swoTerminal4(&customBuf4);
std::ostream swoTerminal5(&customBuf5);
std::ostream swoTerminal6(&customBuf6);
std::ostream swoTerminal7(&customBuf7);

std::ostream swoTerminal8(&customBuf8);
std::ostream swoTerminal9(&customBuf9);
std::ostream swoTerminal10(&customBuf10);
std::ostream swoTerminal11(&customBuf11);
std::ostream swoTerminal12(&customBuf12);
std::ostream swoTerminal13(&customBuf13);
std::ostream swoTerminal14(&customBuf14);
std::ostream swoTerminal15(&customBuf15);