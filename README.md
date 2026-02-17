# ApexServer

A C++ TCP server built using Winsock on Windows.  
Developed in structured phases to deeply understand socket programming and server architecture.

---

## Phase 1 – Basic Echo Server

### Features
- Single-client TCP server
- Localhost communication (127.0.0.1:8000)
- Echo functionality
- Proper Winsock initialization and cleanup

### Server Flow
WSAStartup → socket → bind → listen → accept → recv → send → cleanup

---

## Phase 2 – Persistent Single-Threaded Server

### Improvements
- Continuous accept() loop
- handleClient() abstraction
- Persistent server (does not exit after one client)
- Proper client disconnect detection

### Core Architecture
```cpp
while (true) {
    SOCKET client = accept(serverSocket, nullptr, nullptr);
    handleClient(client);  // blocks per client
}
```

### Behavior
- Handles one client at a time
- Server blocks if a connected client blocks in recv()
- Foundation for multithreading (Phase 3)

---

## Build & Run

### Compile Server
g++ apexserver.cpp -o ApexServer -lws2_32

### Compile Client
g++ apexclient.cpp -o ApexClient -lws2_32

### Run
.\ApexServer.exe  
.\ApexClient.exe  

---

## Next Phase
Phase 3: Thread-per-client concurrency.
