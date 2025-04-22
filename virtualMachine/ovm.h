#include <iostream>
#include <vector>

#pragma once

class OVM
{
public:
    OVM();

    void run();
    void test_input();
private:
    int pc;
    int sp;
    std::vector<int> memory;
    static constexpr size_t MEMORY_SIZE = 8*1024;
    int tickCounter;

    enum operation {
        STOP = -1,
        ADD = -2,
        SUB = -3,
        MUL = -4,
        DIV = -5,
        MOD = -6,
        NEG = -7,
        LOAD = -8,
        SAVE = -9,
        DUP = -10,
        DROP = -11,
        SWAP = -12,
        OVER = -13,
        GOTO = -14,
        IFLT = -15,
        IFLE = -16,
        IFGT = -17,
        IFGE = -18,
        IFEQ = -19,
        IFNE = -20,
        IN = -21,
        OUT = -22,
        LN = -23
    };
};