#include<stdio.h>

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;

int main(){
    uint16_t s,b;
    s = 8;
    b = 8;
    uint64_t x = 0x7fefe05a8;
    uint64_t tag = x >> (s+b);
    uint32_t set = (x >> s) & ((-1u)>>(sizeof(x)*8 - s));
    printf("%x\n%x\n%x",x,tag,set);

}