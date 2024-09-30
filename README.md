# SimpleKV

SimpleKV is a very basic key-value store server, similar in concept to Redis but designed to be much simpler and "dumber". It provides a minimalistic implementation of a key-value store written in C, with both a server and client. SimpleKV operates using a basic, single-threaded protocol for demonstration purposes.

Check out the protocol used between client and server in [PROTOCOL](PROTOCOL.md). It also contains an example of how to implement your own client using the provided `kvstrprotocol.h` functions.

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

- `server.c`: Implements the core key-value store server.
- `kvstore.c` and `kvstore.h`: Implementation of the in-memory key-value-store used by the server.
- `kvstrprotocol.h`: helper functions to implement the [Protocol](PROTOCOL.md) in an application (esp. building requests to send to the server)
- `client.c`: A simple command-line client for testing and interacting with the server.

## Prerequisites

To build and run this project, you will need:
- A Windows environment
- GCC (MinGW or any Windows-compatible GCC distribution)
- Winsock library (`-lws2_32` for linking)
- [Zig](https://www.ziglang.org) for building and experimental tests

## Installation & Compilation
There are no dependencies other than the Winsock library (`-lws2_32`) for networking. So building is quite simple.

### Full Build (with MSYS2 on Windows)
If you have msys2 installed on your machine it is very easy. You can just execute the `ci-build.sh` script that will:

1. Download zig & drmemory
2. Build the `server_test` binary
3. Run the tests
4. Run a memory leak analysis of the tests
5. Build `./zig-out/bin/server`(the server binary) and `./zig-out/bin/client` (the example client binary)

This script is reentrant and cleans up after itself. In case an error comes up, the error message should be kind of self-explanatory.

### Building with `zig build`
The project is set up to be built through `zig build`. To do that:

1. Download and install `zig` from [their website](https://www.ziglang.org/)
2. Run `zig build` in the root folder of the project

The binaries will be built to `./zig-out/bin/`. To verify that everything is working as expected you can run the `server_test` binary that will execute a set of tests that should all pass.

### Building with `make`
In additon, the project also provides a `Makefile`. By default it uses `zig cc` as compiler (see above). Running everything works with:

```shell
make all
```

This will run the tests implicitly. The build output is copied top the `dist` folder in this case.

If you do not want to install and use `zig`, you can also build the project with `gcc` (which will require something line MinGW on windows!):

```shell
make CC="gcc" all
```

## Usage
See the [PROTOCOL](PROTOCOL.md) for a description of the communication protocol used between the `simplekv` server and any client.

### Starting the Server

1. **Run the server:**
    ```sh
    ./server
    ```

   By default, the server listens on port `8080`. This can currently not be changed.

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
