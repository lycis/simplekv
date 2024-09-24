# SimpleKV

SimpleKV is a very basic key-value store server, similar in concept to Redis but designed to be much simpler and "dumber". It provides a minimalistic implementation of a key-value store written in C, with both a server and client. SimpleKV operates using a basic, single-threaded protocol for demonstration purposes.

Check out the protocol used between client and server in [PROTOCOL.md]. It also contains an example of how to implement your own client using the provided `kvstrprotocol.h` functions.

## Purpose

The primary goal of this project is to offer a lightweight, network-enabled key-value store, mainly for quick and easy sharing of state between simple services. **This project should not be used in production or any secure environments.** It was originally written as an exercise to improve my C programming skills, but has since become a useful tool whenever I need a simple, lean, and fast distributed key-value store.

## Features

- **Single-threaded server:** Handles requests one at a time.
- **Basic protocol:** Supports simple `PUT`, `GET` and `DEL` operations.
- **Configurable log levels:** Control log verbosity using command-line arguments.
- **Command-line client:** Provides a minimal interface for interacting with the server.

## Disclaimer

**Windows Only:** Currently, SimpleKV is implemented using WinSock2 for socket programming, so it works exclusively on Windows systems.

## Project Structure

- **`server.c`:** Implements the core key-value store server.
- **`client.c`:** A simple command-line client for testing and interacting with the server.

## Prerequisites

To build and run this project, you will need:
- A Windows environment
- GCC (MinGW or any Windows-compatible GCC distribution)
- Winsock library (`-lws2_32` for linking)

## Installation & Compilation
The project only consists of two primary C files: `server.c` and `client.c`. Both applications are self-ccontained within their files. There are no dependencies other than the Winsock library (`-lws2_32`) for networking. So building is simple:

1. **Clone the repository:**
    ```sh
    git clone https://github.com/yourusername/simplekv.git
    ```

2. **Navigate to the project directory:**
    ```sh
    cd simplekv
    ```

3. **Compile the server:**
    ```sh
    gcc -o server server.c -lws2_32
    ```

4. **Compile the client (optional):**
    ```sh
    gcc -o client client.c -lws2_32
    ```

This will generate the server and client executables (`server.exe` and `client.exe`).

## Usage
See the [PROTOCOL](PROTOCOL.md) for a description of the communication protocol used between the `simplekv` server and any client.

### Starting the Server

1. **Run the server:**
    ```sh
    ./server
    ```

   By default, the server listens on port `8080`.

   The log level can be controlled with the `-l` argument followed by one of the log levels: `ERR`, `INFO`, `DEBUG`, `WARN`, `FATAL`. By default the log level will be set to `INFO`. For example:
    ```sh
    ./server -l DEBUG
    ```

3. **Connect to the server:**
   You can use any TCP client such as Telnet or Netcat to connect to the SimpleKV server. For example, using Telnet:
    ```sh
    telnet localhost 8080
    ```

   Alternatively, you can use the provided SimpleKV client (see below).

### Client Usage (optional)

If you compiled the `client.c`, you can use the SimpleKV client to interact with the server:

1. **Run the client:**
    ```sh
    ./client localhost 8080 put akey keyvalue # store a value on the server
    ./client localhost 8080 get akey # returns '200 keyvalue' from the server (or 404 Not Found)
    ./client localhost 8080 del akey # returns `200 Key deleted` and removes the stored value
    ```

## Contributing

We welcome contributions to make SimpleKV more feature-rich or cross-platform!

1. Fork the repository.
2. Create a new branch:
    ```sh
    git checkout -b feature-branch
    ```
3. Commit your changes:
    ```sh
    git commit -am 'Add new feature'
    ```
4. Push the branch to your fork:
    ```sh
    git push origin feature-branch
    ```
5. Create a new Pull Request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
