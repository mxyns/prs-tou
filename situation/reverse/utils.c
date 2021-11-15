void printNZeros(int n, int maxSeries) {
    
    if (n > 0 && n < maxSeries) {
        for (;n> 0; n--) printf("0 ");
    } else if (n > 0) {
        printf("%d**[0] ", n);
    }

}

void compact_print_buffer(
    char* buffer,
    int size
) {

    printf("[\n\t");    
    int zeros = 0;
    for (int i = 0; i < size; i++) {

        if (buffer[i] == 0) {
            zeros++;

            if (i != size - 1) continue;
        }

        printNZeros(zeros, 5);
        zeros=0;

        if (buffer[i] != 0)
            printf("%#x ", buffer[i]);
    }
    printf("\n]\n");
}