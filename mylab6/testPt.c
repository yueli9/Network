
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void ptrch ( char ** point) {
    //*point = "asd";
    strcpy(*point,"aSD");
}

int main() {
    char * ds;
    ptrch(&ds);
    printf("%s\n", ds);
    return 0;
}
