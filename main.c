#include <WinSock2.h>
#include <stdio.h>
#include <time.h>

enum LogLevel { INFO = 0, WARN = 1, ERR = 2, DEBUG = 3, FATAL = 4 };

void getCurrentTimeString(char *buffer) {
  time_t t = time(NULL);
  struct tm buf;
  char timeStamp[26];
  errno_t err = localtime_s(&buf, &t);
  asctime_s(timeStamp, sizeof timeStamp, &buf);
  timeStamp[24] = '\0';
  strcpy_s(buffer, 25, timeStamp);
  return;
}

const char *getLogLevelAsStr(enum LogLevel l) {
  switch (l) {
  case INFO:
    return "INFO";
  case WARN:
    return "WARN";
  case ERR:
    return "ERR";
  case DEBUG:
    return "DEBUG";
  case FATAL:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

void logMessage(enum LogLevel lvl, const char *message) {
#ifdef WIN64
  char timeStampStr[26];
  getCurrentTimeString(timeStampStr);
  printf("%s - %s - %s\n", timeStampStr, getLogLevelAsStr(lvl), message);
#else
  printf("%s - %s\n", lvlStr, message);
#endif

  if (lvl == FATAL) {
    exit(1);
  }
}

SOCKET createSocket() {
  WSADATA wsaData = {0};
  int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (r != 0) {
    char logBuffer[1024];
    sprintf(logBuffer, "WSAStartup failed. Error code: %d", r);
    logMessage(FATAL, logBuffer);
  }

  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    char logBuffer[1024];
    sprintf(logBuffer, "Failed to create socket. Error code: %d",
            WSAGetLastError());
    logMessage(FATAL, logBuffer);
  }

  int enable = 1;
  r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &enable, sizeof(enable));
  if (r != 0) {
    char logBuffer[1024];
    sprintf(logBuffer, "Failed to set socket options. Error code: %d",
            WSAGetLastError());
    logMessage(FATAL, logBuffer);
  }

  logMessage(DEBUG, "Socket created.");
  return sock;
}

void bindSocket(SOCKET sock, int port) {
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  int r = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  if (r == SOCKET_ERROR) {
    char logBuffer[1024];
    sprintf(logBuffer, "Failed to bind socket. Error code: %d",
            WSAGetLastError());
    logMessage(FATAL, logBuffer);
  }

  logMessage(INFO, "Listening on 0.0.0.0:8080");
}

int main(int argc, char **argv) {
#ifndef WIN64
  printf("This program is only meant to be run on Windows 64-bit. Other OS are "
         "currently not supported.");
#else

  logMessage(INFO, "Starting server.");
  SOCKET sock = createSocket();
  bindSocket(sock, 8080);
  logMessage(INFO, "Server shutdown complete.");

#endif
  return 0;
}