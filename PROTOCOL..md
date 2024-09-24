# Protocol Overview

The communication between the client and server in the `simplekv` system is done using plain text over a TCP socket connection. This protocol is designed to be simple and easy to implement, making it suitable for quick state sharing between services. However, binary data must be encoded (e.g., base64) before transmission, as only text-based communication is supported.

## Key Facts:
- **Plain Text Communication**: All messages are exchanged as plain text.
- **Request Size Limit**: Each request must not exceed **5MB**, which is the buffer size on the server.
- **Request Format**: Every request follows the structure:
  ```
  <operation> <arglen1>:<argvalue1> <arglen2>:<argvalue2> ...
  ```
  Where each argument (`argvalue`) is preceded by its length (`arglen`), ensuring the server knows how much data to read.

## Supported Requests

1. **GET Request**: Retrieves the value of a key.
   - **Example**: 
     ```
     GET 4:akey
     ```
   - **Explanation**: 
     - The `GET` operation requests the value of the key `akey` (length `4`).
   - **Server Response**:
     ```
     200 keyvalue
     ```
     If the key exists, the value (`keyvalue`) is returned. If not, a `404` status is returned.

2. **PUT Request**: Stores a key-value pair.
   - **Example**:
     ```
     PUT 4:akey 7:keyvalue
     ```
   - **Explanation**: 
     - The `PUT` operation stores the value `keyvalue` (length `7`) into the key `akey` (length `4`).
   - **Server Response**:
     ```
     201 Key created
     ```
     If the key is successfully stored, a `201` status is returned. The server will overwrite existing values for the same key.

3. **DEL Request**: Deletes a key from the store.
   - **Example**:
     ```
     DEL 4:akey
     ```
   - **Explanation**: 
     - The `DEL` operation deletes the key `akey` (length `4`).
   - **Server Response**:
     ```
     200 Key deleted
     ```
     If the key exists and is successfully deleted, the server responds with a `200` status. If the key does not exist, a `404` status is returned.

## Response Format

The server responds to every request with a plain text message that follows the structure:
```
<code> <info>
```
- **`<code>`**: Follows HTTP-like status codes:
  - `200`: Successful request
  - `201`: Key successfully created or updated
  - `400`: Malformed or invalid request
  - `404`: Key not found
  - `500`: Internal server error
- **`<info>`**: Context-specific information about the request:
  - For successful `GET` requests, this is the value of the key.
  - For `PUT` and `DEL`, it provides a status message (e.g., "Key created" or "Key deleted").
  - For errors, it provides an error message describing the problem (e.g., "Invalid key length" or "Malformed request").

### Example Responses:
- **GET Success**: `200 keyvalue`
- **PUT Success**: `201 Key created`
- **DEL Success**: `200 Key deleted`
- **Key Not Found (GET)**: `404 Key not found`
- **Bad Request**: `400 Invalid request format`

This simple protocol allows quick and efficient interaction between client and server for basic key-value operations.

## Sequence Diagram for Communication flow

```plaintext
Client                            Server
  |                                  |
  | ---- PUT 4:akey 7:keyvalue ---->  |  // Client sends a PUT request to store a key-value pair
  |                                  |
  | <------ 201 Key created --------- |  // Server responds with success message
  |                                  |
  | ---- GET 4:akey ----------------> |  // Client sends a GET request for the key 'akey'
  |                                  |
  | <------ 200 keyvalue ------------ |  // Server responds with the stored value
  |                                  |
  | ---- DEL 4:akey ----------------> |  // Client sends a DEL request to delete the key
  |                                  |
  | <------ 200 Key deleted --------- |  // Server confirms deletion
  |                                  |
  | ---- GET 4:akey ----------------> |  // Client sends another GET request for 'akey'
  |                                  |
  | <------ 404 Key not found ------- |  // Server responds with 'key not found' message
  |                                  |
```
## Implementing Your Own Client

Any client capable of sending plain text following the protocol outlined above should be compatible with the `simplekv` server. Whether you're writing a client in C, Python, or any other language, as long as you adhere to the request structure, the server will process your commands.

To make the process easier, the project includes a header file, `kvstrprotocol.h`, which provides functions and structs to help you format requests correctly. You can use this header to quickly implement your own client without worrying about manual string formatting.

### Using `kvstrprotocol.h`

When you include `kvstrprotocol.h` in your client code, the following functions are available to help you build the required requests:

- **`char* kvstr_build_get_request(const char* key)`**  
  Creates a properly formatted plain text request for a `GET` operation.
  ```c
  char* request = kvstr_build_get_request("akey");
  send(clientSocket, request, strlen(request), 0);
  ```
  
- **`char* kvstr_build_put_request(const char* key, const char* value)`**  
  Creates a plain text request for a `PUT` operation, storing `value` under `key`.
  ```c
  char* request = kvstr_build_put_request("akey", "keyvalue");
  send(clientSocket, request, strlen(request), 0);
  ```

- **`char* kvstr_build_del_request(const char* key)`**  
  Constructs a request to delete a key using the `DEL` operation.
  ```c
  char* request = kvstr_build_del_request("akey");
  send(clientSocket, request, strlen(request), 0);
  ```

These functions handle the formatting for you, following the `<operation> <arglen1>:<argvalue1> ...` protocol. You just need to pass in the appropriate `key` and `value` strings, and they will output a properly formatted request.

### Example: Writing a Simple Client

Here’s an example of how you could write a simple client in C using the provided functions from `kvstrprotocol.h`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvstrprotocol.h"

int main() {
    // Initialize connection to server (code not shown for brevity)
    
    // Example GET request
    char* getRequest = kvstr_build_get_request("akey");
    send(clientSocket, getRequest, strlen(getRequest), 0);
    
    // Receive response from the server
    char response[1024];
    recv(clientSocket, response, sizeof(response), 0);
    printf("Server response: %s\n", response);
    
    // Example PUT request
    char* putRequest = kvstr_build_put_request("akey", "keyvalue");
    send(clientSocket, putRequest, strlen(putRequest), 0);
    
    // Receive response from the server
    recv(clientSocket, response, sizeof(response), 0);
    printf("Server response: %s\n", response);

    // Free any allocated memory if needed
    free(getRequest);
    free(putRequest);
    
    // Close connection (code not shown)
    return 0;
}
```

This example demonstrates how easy it is to interact with the `simplekv` server when using the provided utility functions. You can adapt this approach to any programming language or environment, as long as you ensure that your client sends and receives plaintext messages conforming to the protocol.