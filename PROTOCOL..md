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
