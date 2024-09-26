#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvstrprotocol.h"

#ifdef WIN64
#include <WinSock2.h>
#endif

void getFromServer(char *server, int port, char *key) {
  WSADATA wsaData = {0};
  int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (r != 0) {
    char logBuffer[1024];
    printf(logBuffer, "WSAStartup failed. Error code: %d", r);
  }

  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    printf("Failed to create socket: %d\n", WSAGetLastError());
    exit(1);
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(8080);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
  int rv = connect(sock, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    printf("Failed to connect: %d\n", WSAGetLastError());
    exit(1);
  }

  char* msg = kvstr_build_get_request(key);
  send(sock, msg, strlen(msg), 0);
  free(msg);

  char rbuf[64] = {};
  size_t n = recv(sock, rbuf, sizeof(rbuf) - 1, 0);
  if (n < 0) {
    printf("Failed to receive: %d\n", WSAGetLastError());
    exit(1);
  }
  printf("server says: %s\n", rbuf);
  closesocket(sock);
}

void delFromServer(char *server, int port, char *key) {
  WSADATA wsaData = {0};
  int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (r != 0) {
    char logBuffer[1024];
    printf(logBuffer, "WSAStartup failed. Error code: %d", r);
  }

  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    printf("Failed to create socket: %d\n", WSAGetLastError());
    exit(1);
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(8080);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
  int rv = connect(sock, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    printf("Failed to connect: %d\n", WSAGetLastError());
    exit(1);
  }

  char* msg = kvstr_build_del_request(key);
  send(sock, msg, strlen(msg), 0);
  free(msg);

  char rbuf[64] = {};
  size_t n = recv(sock, rbuf, sizeof(rbuf) - 1, 0);
  if (n < 0) {
    printf("Failed to receive: %d\n", WSAGetLastError());
    exit(1);
  }
  printf("%s\n", rbuf);
  closesocket(sock);
}


void setValueOnServer(char *server, int port, char *key, char *value) {
    WSADATA wsaData = {0};
  int r = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (r != 0) {
    char logBuffer[1024];
    printf(logBuffer, "WSAStartup failed. Error code: %d", r);
  }

  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    printf("Failed to create socket: %d\n", WSAGetLastError());
    exit(1);
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(8080);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
  int rv = connect(sock, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    printf("Failed to connect: %d\n", WSAGetLastError());
    exit(1);
  }

  char* msg = kvstr_build_put_request(key, value);
  send(sock, msg, strlen(msg), 0);
  free(msg);

  char rbuf[64] = {};
  size_t n = recv(sock, rbuf, sizeof(rbuf) - 1, 0);
  if (n < 0) {
    printf("Failed to receive: %d\n", WSAGetLastError());
    exit(1);
  }
  printf("server says: %s\n", rbuf);
  closesocket(sock);
}

int main(int argc, char **argv) {
#ifndef WIN64
  printf("This program is only meant to be run on Windows 64-bit. Other OS are "
         "currently not supported.");
#endif

  if (argc < 5) {
    printf("Usage: %s <server> <port> <GET key | SET key value>\n", argv[0]);
    return 1;
  }

  char *server = argv[1];
  int port = atoi(argv[2]);
  char *command = argv[3];
  if (strcmp(command, "GET") == 0 || strcmp(command, "get") == 0) {
    char *key = argv[4];
    getFromServer(server, port, key);
  } else if (strcmp(command, "PUT") == 0 || strcmp(command, "put") == 0) {
    char *key = argv[4];
    if (argc < 6) {
      printf("usage: SET <key> <value>\n");
      return 1;
    }
    char *value = argv[5];
    setValueOnServer(server, port, key, value);
  } else if(strcmp(command, "DEL") == 0 || strcmp(command, "del") == 0) {
    char *key = argv[4];
    delFromServer(server, port, key);
  } else {
    printf("Invalid command\n");
  }

  return 0;
}