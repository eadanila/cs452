#ifndef FRAME_H
#define FRAME_H

struct __attribute__((__packed__)) frame {
    uint cspr;
    uint r0;
    uint r1;
    uint r2;
    uint r3;
    uint r4;
    uint r5;
    uint r6;
    uint r7;
    uint r8;
    uint r9;
    uint r10;
    uint r11;
    uint r12;
    uint r13;
    uint r14;
    uint r15;
    uint stk0;
    uint stk1;
};
typedef struct frame Frame;

#endif

