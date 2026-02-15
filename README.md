ENDGOAL is to make a multi-threaded C++ server which can accept 1000+ requests simultaneously

# ApexServer

A C++ TCP server built using Winsock on Windows.

## Phase 1 - Basic Echo Server

### Features

- Single-client TCP server
- Client-server communication over localhost
- Echo functionality
- Proper Winsock initialization and cleanup

### Architecture

Server:

- WSAStartup
- socket()
- bind()
- listen()
- accept()
- recv()
- send()

Client:

- WSAStartup
- socket()
- connect()
- send()
- recv()

### How to Run

1. Compile server:
   g++ apexserver.cpp -o ApexServer -lws2_32

2. Compile client:
   g++ apexclient.cpp -o ApexClient -lws2_32

3. Run server:
   .\ApexServer.exe

4. Run client in separate terminal:
   .\ApexClient.exe
