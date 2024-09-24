#include "kvstrprotocol.h"
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

#ifdef WIN64
#include <WinSock2.h>
#endif

enum LogLevel {
  FATAL = 0,
  WARN = 1,
  ERR = 2,
  INFO = 3,
  DEBUG = 4,
};

/*** global variables start ***/
static enum LogLevel gl_logLevel = INFO;
static volatile bool gl_keepRunning = true;
static volatile bool gl_cleanedUp = false;
static SOCKET gl_serverSocket;
/*** global variables end ***/

/*** prototypes ***/
void handleConnections(SOCKET serverSocket);
SOCKET acceptClientConnection(SOCKET serverSocket, char* logBuffer, size_t logBufferSize);
void handleAcceptError(char* logBuffer, size_t logBufferSize);
int receiveData(SOCKET clientSocket, char* buffer, size_t bufferSize, char* logBuffer, size_t logBufferSize);
void processClientRequest(SOCKET clientSocket, char* buffer, char* logBuffer, size_t logBufferSize);
void handleGetRequest(SOCKET clientSocket, const char* key, char* logBuffer, size_t logBufferSize);
void handlePutRequest(SOCKET clientSocket, const char* key, const char* value, char* logBuffer, size_t logBufferSize);
void handleDelRequest(SOCKET clientSocket, const char* key, char* logBuffer, size_t logBufferSize);


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
  if (gl_logLevel < lvl) {
    return;
  }

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

void setLoglevel(const char *lvl) {
  if (strcmp(lvl, "INFO") == 0) {
    gl_logLevel = INFO;
  } else if (strcmp(lvl, "WARN") == 0) {
    gl_logLevel = WARN;
  } else if (strcmp(lvl, "ERR") == 0) {
    gl_logLevel = ERR;
  } else if (strcmp(lvl, "DEBUG") == 0) {
    gl_logLevel = DEBUG;
  } else if (strcmp(lvl, "FATAL") == 0) {
    gl_logLevel = FATAL;
  } else {
    logMessage(WARN, "Unknown log level. Defaulting to INFO.");
    gl_logLevel = INFO;
  }

  char logBuffer[1024];
  sprintf(logBuffer, "Log level set to %s.", getLogLevelAsStr(gl_logLevel));
  logMessage(INFO, logBuffer);

  return;
}

void createSocket() {
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
  r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&enable,
                 sizeof(enable));
  if (r != 0) {
    char logBuffer[1024];
    sprintf(logBuffer, "Failed to set socket options. Error code: %d",
            WSAGetLastError());
    logMessage(FATAL, logBuffer);
  }

  logMessage(DEBUG, "Socket created.");
  gl_serverSocket = sock;
}

void bindSocket(SOCKET sock, int port) {
  char logBuffer[1024];
  int r;

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  r = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  if (r == SOCKET_ERROR) {
    memset(logBuffer, 0, sizeof(logBuffer));
    sprintf(logBuffer, "Failed to bind socket. Error code: %d",
            WSAGetLastError());
    logMessage(FATAL, logBuffer);
  }

  r = listen(sock, SOMAXCONN);
  if (r != 0) {
    memset(logBuffer, 0, sizeof(logBuffer));
    sprintf(logBuffer, "Failed to listen on socket. Error code: %d",
            WSAGetLastError());
    logMessage(FATAL, logBuffer);
  }
  logMessage(INFO, "Listening on 0.0.0.0:8080");
}

void handleConnections(SOCKET serverSocket) {
    char logBuffer[1024];
    
    // Main loop for accepting connections
    while (gl_keepRunning) {
        SOCKET clientSocket = acceptClientConnection(serverSocket, logBuffer, sizeof(logBuffer));
        if (clientSocket == INVALID_SOCKET) {
            continue;  // If accept fails, continue to the next iteration
        }

        char buffer[2048] = {0};
        if (receiveData(clientSocket, buffer, sizeof(buffer), logBuffer, sizeof(logBuffer)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            continue;  // Move to the next iteration in case of receiving error
        }

        // Process the request
        processClientRequest(clientSocket, buffer, logBuffer, sizeof(logBuffer));

        // Close the client connection
        closesocket(clientSocket);
    }
}

SOCKET acceptClientConnection(SOCKET serverSocket, char* logBuffer, size_t logBufferSize) {
    struct sockaddr_in clientAddr;
    size_t clientAddrSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, (int*)&clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        handleAcceptError(logBuffer, logBufferSize);
        return INVALID_SOCKET;
    }

    // Log successful connection
    snprintf(logBuffer, logBufferSize, "Accepted connection from %s:%d",
             inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    logMessage(DEBUG, logBuffer);

    return clientSocket;
}

void handleAcceptError(char* logBuffer, size_t logBufferSize) {
    int errorCode = WSAGetLastError();
    if (errorCode == WSAEINTR) {
        logMessage(INFO, "Received interrupt signal. Stopping new connections.");
    } else {
        snprintf(logBuffer, logBufferSize, "Failed to accept connection. Error code: %d", errorCode);
        logMessage(ERR, logBuffer);
    }
}

int receiveData(SOCKET clientSocket, char* buffer, size_t bufferSize, char* logBuffer, size_t logBufferSize) {
    int receivedBytes = recv(clientSocket, buffer, bufferSize, 0);
    if (receivedBytes == SOCKET_ERROR) {
        snprintf(logBuffer, logBufferSize, "Failed to receive data. Error code: %d", WSAGetLastError());
        logMessage(ERR, logBuffer);
    }
    return receivedBytes;
}

void processClientRequest(SOCKET clientSocket, char* buffer, char* logBuffer, size_t logBufferSize) {
    struct kvstr_request* req = create_kvstr_request();
    kvstr_parse_request(buffer, req);

    if (strcmp(req->operation, "GET") == 0) {
        handleGetRequest(clientSocket, req->key, logBuffer, logBufferSize);
    } else if (strcmp(req->operation, "PUT") == 0) {
        handlePutRequest(clientSocket, req->key, req->value, logBuffer, logBufferSize);
    } else if (strcmp(req->operation, "DEL") == 0) {
        handleDelRequest(clientSocket, req->key, logBuffer, logBufferSize);
    } else {
        logMessage(WARN, "Received unknown request.");
    }

    free_kvstr_request(req);
}

void handleGetRequest(SOCKET clientSocket, const char* key, char* logBuffer, size_t logBufferSize) {
    snprintf(logBuffer, logBufferSize, "Received GET request for key: %s", key);
    logMessage(INFO, logBuffer);

    // Send a dummy value for now
    const char* response = "dummy_value";
    send(clientSocket, response, strlen(response), 0);
}

void handlePutRequest(SOCKET clientSocket, const char* key, const char* value, char* logBuffer, size_t logBufferSize) {
    snprintf(logBuffer, logBufferSize, "Received PUT request for key: %s and value: %s", key, value);
    logMessage(INFO, logBuffer);

    // Send an acknowledgment
    char response[1024];
    snprintf(response, sizeof(response), "PUT %s", key);
    send(clientSocket, response, strlen(response), 0);
}

void handleDelRequest(SOCKET clientSocket, const char* key, char* logBuffer, size_t logBufferSize) {
    snprintf(logBuffer, logBufferSize, "Received DEL request for key: %s", key);
    logMessage(INFO, logBuffer);

    // Confirm deletion (dummy implementation for now)
    char response[1024];
    snprintf(response, sizeof(response), "DEL %s", key);
    send(clientSocket, response, strlen(response), 0);
}

void cleanUp() {
    if(gl_cleanedUp) {
        return;
    }
    closesocket(gl_serverSocket);
    WSACleanup();
    gl_cleanedUp = true;
    logMessage(INFO, "Graceful clean up done.");
    return;
}

void handleInterrupt(int signal) {
  logMessage(INFO, "Received interrupt signal. Shutting down server.");
  gl_keepRunning = false;
  cleanUp();
}

int main(int argc, char **argv) {
#ifndef WIN64
  printf("This program is only meant to be run on Windows 64-bit. Other OS are "
         "currently not supported.");
#else

  signal(SIGINT, handleInterrupt);

  if (argc > 1) {
    if (argc < 3) {
      logMessage(WARN,
                 "Invalid number of arguments. Usage: server [-l loglevel]");
      return 1;
    }

    // check if -l is given and set global log level
    if (strcmp(argv[1], "-l") == 0) {
      setLoglevel(argv[2]);
    }
  }

  logMessage(INFO, "Starting server.");
  createSocket();
  bindSocket(gl_serverSocket, 8080);
  handleConnections(gl_serverSocket);
  cleanUp();
  logMessage(INFO, "Server shutdown complete.");

#endif
  return 0;
}