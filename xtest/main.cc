#include <stdio.h>

extern "C" long output();

extern "C" int main() {
    printf("output: %ld\n", output());
}