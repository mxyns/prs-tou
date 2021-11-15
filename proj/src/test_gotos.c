#include <stdio.h>

int f() {

    if (1) {
        printf("start\n");
    }

    early_stop : {

        printf("early_stop\n");
        return 1;
    }
    printf("in between\n");
    error : {

        printf("error\n");
        goto early_stop;
    }

}

int main() {

    f();
}