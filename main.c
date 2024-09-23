#include <stdio.h>
#include <WinSock2.h>
#include <time.h>

void logMessage(int level, const char* message) {
    #ifdef WIN64
        time_t t = time(NULL);
        struct tm buf;
        char str[26];
        errno_t err = localtime_s(&buf, &t);    
        asctime_s(str, sizeof str, &buf);
        str[24] = '\0';
        printf("%s: %s\n", str, message);
    #else
        printf("%s\n", message);
    #endif
}

int main() {
    logMessage(0, "Starting ckvstr.");

    //int fd = socket(AF_INET, SOCK_STREAM, 0);

    return 0;
}