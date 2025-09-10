#pragma once

#include <iostream>


class SWOStreamBuffer : public std::streambuf {
   public:
    explicit SWOStreamBuffer(int port) : port(port) {}

   protected:
    virtual int overflow(int c) override;

   private:
    int port;
};
