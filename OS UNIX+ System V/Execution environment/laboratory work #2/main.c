#include <stdio.h>  // for printf
#include <time.h>   //for time
#include <stdlib.h> //for putenv
#define MAX_SIZE 128

int main() {
    putenv("TZ=America/Los_Angeles");
    time_t now;
    struct tm *sp;

    time(&now);

    /* deprecated */
    printf("%s", ctime(&now));

    sp = localtime(&now);
    char* buff = malloc(sizeof(char) * MAX_SIZE);
    if(strftime(buff, MAX_SIZE, "%c", sp) != 0) {
        printf("%s\n", buff);
    }

    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);
    return 0;
}