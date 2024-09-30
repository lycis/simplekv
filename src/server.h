#ifndef _H_KVSTORE_SERVER_H
#define _H_KVSTORE_SERVER_H

#ifdef _WIN64
#include <WinSock2.h>
#endif

/* Data Types */
enum LogLevel {
  FATAL = 0,
  WARN = 1,
  ERR = 2,
  INFO = 3,
  DEBUG = 4,
};

// represents a request from the client
struct kvstr_request {
    char* key;
    char* value;
    char* operation;
};


/* Prototypes */
void handleConnections(SOCKET serverSocket);
SOCKET acceptClientConnection(SOCKET serverSocket, char *logBuffer,
                              size_t logBufferSize);
void handleAcceptError(char *logBuffer, size_t logBufferSize);
int receiveData(SOCKET clientSocket, char *buffer, size_t bufferSize);
void processClientRequest(SOCKET clientSocket, char *buffer, char *logBuffer,
                          size_t logBufferSize);
void handleGetRequest(SOCKET clientSocket, const char *key);
void handlePutRequest(SOCKET clientSocket, const char *key, const char *value);
void handleDelRequest(SOCKET clientSocket, const char *key);
const char* parse_value(const char *after_key_ptr, struct kvstr_request *result);
int kvstr_parse_request(const char *request_str, struct kvstr_request *result);
const char *parse_operation(const char *request_str, struct kvstr_request *result);
const char *parse_key(const char *after_op_ptr, struct kvstr_request *result);

#endif