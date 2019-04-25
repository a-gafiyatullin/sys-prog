//
// Created by xp10rd on 4/22/19.
//
#include <stdio.h>
#include <ctype.h>

int main() {
    int symbol;

    while((symbol = getc(stdin)) != EOF) {
        putc(toupper(symbol), stdout);
    }

    return 0;
}