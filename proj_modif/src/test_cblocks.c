#include <stdio.h>

int main() {

    int a = 0;
    int b = 1;
    printf("before c-block : a=%d b=%d\n", a, b);
    {
        printf("in c-block before new a : a=%d b=%d\n", a, b);
        int a = a + b;
        b = 2;
        printf("in c-block before after new a & b : a=%d b=%d\n", a, b);
    }
    printf("after c-block : a=%d b=%d\n", a, b);
}