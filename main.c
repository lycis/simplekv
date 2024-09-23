#include <stdio.h>
#include <WinSock2.h>
#include <time.h>

enum LogLevel {
    INFO = 0,
    WARN = 1,
    ERR = 2
};

void getCurrentTimeString(char* buffer) {
    time_t t = time(NULL);
    struct tm buf;
    char timeStamp[26];
    errno_t err = localtime_s(&buf, &t);    
    asctime_s(timeStamp, sizeof timeStamp, &buf);
    timeStamp[24] = '\0';
    strcpy_s(buffer, 25, timeStamp);
    return;
}

const char* getLogLevelAsStr(enum LogLevel l) {
    switch(l) {
        case INFO:
            return "INFO";
        case WARN:
            return "WARN";
        case ERR:
            return "ERR";
        default:
            return "UNKNOWN";
    }
}

void logMessage(enum LogLevel lvl, const char* message) {
    #ifdef WIN64
        char timeStampStr[26];
        getCurrentTimeString(timeStampStr);
        printf("%s - %s - %s\n", timeStampStr, getLogLevelAsStr(lvl), message);
    #else
        printf("%s - %s\n", lvlStr, message);
    #endif
}

int main() {
    logMessage(INFO, "Starting server.");

    //int fd = socket(AF_INET, SOCK_STREAM, 0);

    logMessage(INFO, "Server shutdown complete.");


    return 0;
}