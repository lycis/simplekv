#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include "kvstore.h"
#include "server.h"

#define SKVS_SERVER

#ifdef UNIT_TEST
#include "utilfuns.h"

/* very bad c mocking :) */
const char* _mock_lastMessage = NULL;

int mock_send(SOCKET socket, const char *buffer, size_t length, int flags) {
    // Simulate sending a message; you can log it or store it for assertions later
    if(_mock_lastMessage != NULL) {
        free((void*) _mock_lastMessage);
    }
    _mock_lastMessage = duplicate_string(buffer);
    return length; // Simulate successful send
}

#define send mock_send
#endif

/*** global variables start ***/
static enum LogLevel gl_logLevel = INFO;
static volatile bool gl_keepRunning = true;
static volatile bool gl_cleanedUp = false;
static SOCKET gl_serverSocket;
kv_store* gl_kvStore;
/*** global variables end ***/

#define MAX_REQUEST_SIZE 5 * 1024 * 1024 // 5 MB buffer for requests

// helper fucntion to free the memory allocated for the request
void free_kvstr_request(struct kvstr_request** req_ptr) {
  if (req_ptr == NULL) {
      return;
  }

  struct kvstr_request* req = *req_ptr;
  if (req == NULL) {
      return;
  }
  
  if (req->operation != NULL) {
      free(req->operation);
      req->operation = NULL;
  }
  if (req->key != NULL) {
      free(req->key);
      req->key = NULL;
  }
  if (req->value != NULL) {
      free(req->value);
      req->value = NULL;
  }

  free(req);
  *req_ptr = NULL;
}

// helper function to initialize a clean request struct
struct kvstr_request* create_kvstr_request() {
    struct kvstr_request* req = (struct kvstr_request*) malloc(sizeof(struct kvstr_request));
    if (req == NULL) {
        return NULL;  // Memory allocation failure
    }

    req->operation = NULL;
    req->key = NULL;
    req->value = NULL;

    return req;
}

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

#ifndef UNIT_TEST
void logMessage(enum LogLevel lvl, const char *message) {
  if (gl_logLevel < lvl) {
    return;
  }

#ifdef _WIN64
  char timeStampStr[26];
  getCurrentTimeString(timeStampStr);
  printf("%s - %s - %s\n", timeStampStr, getLogLevelAsStr(lvl), message);
#else
  printf("%s - %s\n", getLogLevelAsStr(lvl), message);
#endif

  if (lvl == FATAL) {
    exit(1);
  }
}
#else
void logMessage(enum LogLevel lvl, const char *message) { 
  // no logging during tests
}
#endif

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

  // we keep running as long as there was no interrupt
  while (gl_keepRunning) {
    SOCKET clientSocket =
        acceptClientConnection(serverSocket, logBuffer, sizeof(logBuffer));
    if (clientSocket == INVALID_SOCKET) {
      continue; // If accept fails, continue to the next iteration
    }

    char* buffer = (char*)calloc(MAX_REQUEST_SIZE, 1);
    if (receiveData(clientSocket, buffer, MAX_REQUEST_SIZE) == SOCKET_ERROR) {
      closesocket(clientSocket);
      continue; // Move to the next iteration in case of receiving error
    }

    processClientRequest(clientSocket, buffer, logBuffer, sizeof(logBuffer));

    free(buffer); // release message buffer
    closesocket(clientSocket);
  }
}

SOCKET acceptClientConnection(SOCKET serverSocket, char *logBuffer,
                              size_t logBufferSize) {
  struct sockaddr_in clientAddr;
  size_t clientAddrSize = sizeof(clientAddr);

  SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr,
                               (int *)&clientAddrSize);
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

void handleAcceptError(char *logBuffer, size_t logBufferSize) {
  int errorCode = WSAGetLastError();
  if (errorCode == WSAEINTR) {
    logMessage(INFO, "Received interrupt signal. Stopping new connections.");
  } else {
    snprintf(logBuffer, logBufferSize,
             "Failed to accept connection. Error code: %d", errorCode);
    logMessage(ERR, logBuffer);
  }
}

int receiveData(SOCKET clientSocket, char *buffer, size_t bufferSize) {
  int receivedBytes = recv(clientSocket, buffer, bufferSize, 0);
  if (receivedBytes == SOCKET_ERROR) {
    char logBuffer[1024];
    snprintf(logBuffer, sizeof(logBuffer), "Failed to receive data. Error code: %d",
             WSAGetLastError());
    logMessage(ERR, logBuffer);
  }
  return receivedBytes;
}

int kvstr_parse_request(const char *request_str, struct kvstr_request *result) {
  if (request_str == NULL || result == NULL) {
    return -1; // Invalid input
  }

  result->operation = result->key = result->value = NULL;

  const char *after_op_ptr = parse_operation(request_str, result);
  if (after_op_ptr == NULL) {
    return -2; // Failed to parse operation
  }


  const char *after_key_ptr = parse_key(after_op_ptr, result);
  if (after_key_ptr == NULL) {
    return -3; // Failed to parse key
  }

  // If this is a PUT request, parse the value
  if (strcmp(result->operation, "PUT") == 0) {
    after_key_ptr = parse_value(after_key_ptr, result);
    if (after_key_ptr == NULL) {
      return -4; // Failed to parse value
    }
  }

  // Ensure that the request string has been fully parsed
  if (*after_key_ptr != '\0') {
    return -5; // Junk data found after parsing
  }
  
  return 0; // Success
}

// Helper function to parse the operation from the request
const char *parse_operation(const char *request_str,
                            struct kvstr_request *result) {
  const char *space_ptr = strchr(request_str, ' ');
  if (!space_ptr) {
    return NULL; // Malformed request (no space found)
  }

  int operation_len = space_ptr - request_str;
  result->operation = (char *)malloc(operation_len + 1);
  if (!result->operation) {
    return NULL;
  }

  strncpy(result->operation, request_str, operation_len);
    result->operation[operation_len] = '\0'; // Null-terminate

  if(strcmp(result->operation, "GET") != 0 && strcmp(result->operation, "PUT") != 0 && strcmp(result->operation, "DEL") != 0) {
    free(result->operation);
    result->operation = NULL;
    return NULL; // Invalid operation
  }

  return space_ptr + 1; // Return pointer to next part of the string
}

const char *parse_key(const char *after_op_ptr, struct kvstr_request *result) {
  char *colon_ptr = strchr(after_op_ptr, ':');
  if (!colon_ptr) {
    return NULL; // Malformed request (no colon found)
  }

  int key_len = atoi(after_op_ptr);  // Convert key length to integer
  if (key_len <= 0) {
    return NULL; // Invalid key length
  }

  const char *key_ptr = colon_ptr + 1;
  
  // Ensure the input string is long enough for the key
  if (strlen(key_ptr) < key_len) {
    return NULL; // Key length mismatch
  }

  result->key = (char *)malloc(key_len + 1); // Allocate memory for the key
  if (!result->key) {
    return NULL; // Memory allocation failure
  }

  // Copy the key and ensure null-termination
  strncpy(result->key, key_ptr, key_len);
  result->key[key_len] = '\0'; // Manually null-terminate

  // Additional validation: check if the key matches the expected length
  if (strlen(result->key) != key_len) {
    free(result->key);
    return NULL; // Actual key length does not match the given length
  }

  return key_ptr + key_len; // Return pointer to next part of the string
}

// Helper function to parse the value from the request (for PUT)
const char* parse_value(const char *after_key_ptr, struct kvstr_request *result) {
  if (*after_key_ptr != ' ') {
    return NULL; // Malformed request (no space after key)
  }

  after_key_ptr++; // Skip the space

  char *value_colon_ptr = strchr(after_key_ptr, ':');
  if (!value_colon_ptr) {
    return NULL; // Malformed request (no colon found for value length)
  }

  int value_len = atoi(after_key_ptr); // Parse value length
  if (value_len <= 0) {
    return NULL; // Invalid value length
  }

  const char *value_ptr = value_colon_ptr + 1;

  // Ensure the input string is long enough for the value
  if (strlen(value_ptr) < value_len) {
    return NULL; // Value length mismatch
  }

  // Allocate memory for the value and ensure null-termination
  result->value = (char *)malloc(value_len + 1); // +1 for null terminator
  if (!result->value) {
    return NULL; // Memory allocation failure
  }

  strncpy(result->value, value_ptr, value_len); // Copy the value
  result->value[value_len] = '\0'; // Null-terminate the value

  // Check if the copied value has the expected length
  if (strlen(result->value) != value_len) {
    free(result->value);
    return NULL; // Actual value length does not match the given length
  }

  return value_ptr + value_len;
}


const char *parseError2str(int error) {
  switch (error) {
  case -1:
    return "invalid input";
  case -2:
    return "malformed operation";
  case -3:
    return "malformed key";
  case -4:
    return "malformed value";
  default:
    return "Unknown error";
  }
}

void processClientRequest(SOCKET clientSocket, char *buffer, char *logBuffer,
                          size_t logBufferSize) {
  struct kvstr_request *req = create_kvstr_request();

  int parseRequestError = kvstr_parse_request(buffer, req);
  if (parseRequestError != 0) {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "400 Bad Request: %s", parseError2str(parseRequestError));
    logMessage(ERR, buffer);
    send(clientSocket, buffer, strlen(buffer), 0);
  } else {
    if (strcmp(req->operation, "GET") == 0) {
      handleGetRequest(clientSocket, req->key);
    } else if (strcmp(req->operation, "PUT") == 0) {
      handlePutRequest(clientSocket, req->key, req->value);
    } else if (strcmp(req->operation, "DEL") == 0) {
      handleDelRequest(clientSocket, req->key);
    } else {
      logMessage(ERR, "Received unknown request.");
    }
  }

  free_kvstr_request(&req);
  return;
}

void logKvStoreStatus() {
  // return a status of kv store statistics (current stored entries and capcaity)
  char buffer[1024];
  snprintf(buffer, 1024, "kvstore status -> size='%d' capacity='%d'", (int) gl_kvStore->size, (int) gl_kvStore->capacity);
  logMessage(DEBUG, buffer);
  return;
}

void handleGetRequest(SOCKET clientSocket, const char *key) {
  char logBuffer[1024];
  size_t logBufferSize = sizeof(logBuffer);

  if(key == NULL) {
    logMessage(ERR, "Invalid GET request: Key is NULL.");
    char *errMsg = "400 Bad Request: No key";
    send(clientSocket, errMsg, strlen(errMsg), 0);
    return;
  }

  if(strlen(key) < 1) {
    logMessage(ERR, "Invalid GET request: Key is empty.");
    char *errMsg = "400 Bad Request: No key";
    send(clientSocket, errMsg, strlen(errMsg), 0);
    return;
  }

  snprintf(logBuffer, logBufferSize, "Received GET request for key: %s", key);
  logMessage(INFO, logBuffer);

  const char *value = kv_store_get(gl_kvStore, key);
  if(value == NULL) {
    memset(logBuffer, 0, logBufferSize);
    sprintf(logBuffer, "Key '%s' not found.", key);
    logMessage(INFO, logBuffer);
    char *errMsg = "404 Not Found";
    send(clientSocket, errMsg, strlen(errMsg), 0);
    return;
  }

  char* response = calloc(strlen(value) + 5, 1);
  sprintf(response, "200 %s", value);
  
  send(clientSocket, response, strlen(response), 0);

  free((void*) response);

  return;
}

void handlePutRequest(SOCKET clientSocket, const char *key, const char *value) {
  char logBuffer[1024];
  if (key == NULL || value == NULL) {
      const char *errorMsg = "500 Internal Server Error: Key and value must not be NULL.";
      send(clientSocket, errorMsg, strlen(errorMsg), 0);
      logMessage(ERR, "Invalid PUT request: Key or value is NULL.");
      return;
  }

  if (strlen(key) < 1 || strlen(value) < 1) {
    const char *errorMsg = "400 Bad Request: Key and value must not be empty.";
      send(clientSocket, errorMsg, strlen(errorMsg), 0);
      logMessage(ERR, "Invalid PUT request: Key or value is empty.");
      return;
  }

  int result = kv_store_put(gl_kvStore, key, value);
  if (result != 0) {
    memset(logBuffer, 0, 1024);
    snprintf(logBuffer, 1024, "Failed to store key: %s, reason: %d", key, result);
    logMessage(ERR, logBuffer);
    char response[256];
    snprintf(response, sizeof(response), "500 Internal Server Error: Failed to store key: %s, reason: %d", key, result);
    send(clientSocket, response, strlen(response), 0);
    return;
  }

  memset(logBuffer, 0, 1024);
  snprintf(logBuffer, 1024, "Key '%s' stored successfully.", key);
  logMessage(INFO, logBuffer);
  const char *successMsg = "201 Created: Key stored successfully.";
  send(clientSocket, successMsg, strlen(successMsg), 0);
}


void handleDelRequest(SOCKET clientSocket, const char *key) {
  char logBuffer[1024];
  size_t logBufferSize = sizeof(logBuffer);

  if(key == NULL) {
    logMessage(ERR, "Invalid DEL request: Key is NULL.");
    char *errMsg = "400 Bad Request: No key";
    send(clientSocket, errMsg, strlen(errMsg), 0);
    return;
  }

  if(strlen(key) < 1) {
    logMessage(ERR, "Invalid DEL request: Key is empty.");
    char *errMsg = "400 Bad Request: No key";
    send(clientSocket, errMsg, strlen(errMsg), 0);
    return;
  }

  snprintf(logBuffer, logBufferSize, "Received DEL request for key: %s", key);
  logMessage(INFO, logBuffer);

  if(kv_store_delete(gl_kvStore, key) != 0) {
    memset(logBuffer, 0, logBufferSize);
    sprintf(logBuffer, "Key '%s' not found.", key);
    logMessage(INFO, logBuffer);
    char *errMsg = "404 Not Found";
    send(clientSocket, errMsg, strlen(errMsg), 0);
    return;
  }

  // Confirm deletion (dummy implementation for now)
  char response[1024];
  snprintf(response, sizeof(response), "200 Key deleted");
  send(clientSocket, response, strlen(response), 0);
}

void cleanUp() {
  if (gl_cleanedUp) {
    return;
  }
  
  closesocket(gl_serverSocket);
  WSACleanup();

  if(gl_kvStore != NULL) {
    free_kv_store(gl_kvStore);
    gl_kvStore = NULL;
    logMessage(DEBUG, "Key value store freed.");
  }

  gl_cleanedUp = true;
  logMessage(INFO, "Graceful clean up done.");
  return;
}

void handleInterrupt(int signal) {
  logMessage(INFO, "Received interrupt signal. Shutting down server.");
  gl_keepRunning = false;
  cleanUp();
}

#ifndef UNIT_TEST // in case of unit tests the server_unit_tests.c will be the entry point
int main(int argc, char **argv) {
#ifndef _WIN64
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

  // Initialize the key value store
  logMessage(INFO, "Initializing key value store with initial capacity of 1024");
  gl_kvStore = create_kv_store(1024);

  // Bind and listen on the server socket
  createSocket();
  bindSocket(gl_serverSocket, 8080);
  handleConnections(gl_serverSocket);

  cleanUp();
  logMessage(INFO, "Server shutdown complete.");

#endif
  return 0;
}
#endif