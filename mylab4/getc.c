#include <stdio.h>
#include <stdlib.h>
int main() {
    int c;                 /* int */
    int count;
    char arr[50];
    bzero(arr,50);
    c = getchar();
    count = 0;
    while ((count < 50) && (c != EOF)&& (c != '\n')) {    /* don't go over the array size! */
        arr[count] = c;
        ++count;
        c = getchar();     /* get *another* character */
    }
    printf("output is :\n");
    printf("%s\n",arr );
    return (EXIT_SUCCESS);
}