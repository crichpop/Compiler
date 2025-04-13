#include "iostream"
#pragma once

class Driver
{
public:
    static char ch;
    static unsigned int line;
    static unsigned int position;
    static char chSpace;
    static char chTab;
    static char chEOL;
    static char chEOT;

    Driver() = default;
    void resetText(char const *path);
    void nextCh();

private:
    static std::vector<char> inputChain;
};