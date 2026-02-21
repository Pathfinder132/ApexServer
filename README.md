# ApexServer

A C++ TCP chat server built using Winsock on Windows.  
Developed in structured phases to deeply understand socket programming, blocking behavior, multithreading, and concurrent server architecture.

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

- Continuous `accept()` loop
- `handleClient()` abstraction
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
- Server blocks if a client blocks in `recv()`
- Establishes foundation for concurrency

---

## Phase 3 – Thread-Per-Client Concurrency

### Improvements

- Each client handled in its own `std::thread`
- Main thread remains free to accept new connections
- True concurrent client handling

### Core Architecture

```cpp
while (true) {
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    std::thread clientThread(handleClient, clientSocket, clientID);
    clientThread.detach();
}
```

### Behavior

- Multiple clients can stay connected simultaneously
- Blocking `recv()` occurs per client thread, not per server
- Introduces OS-level concurrency

---

## Phase 4 – Real-Time Broadcast Chat System

### Improvements

- Shared `std::vector<SOCKET>` to track active clients
- `std::mutex` for thread-safe client management
- Real-time message broadcasting to all connected clients
- Safe client removal on disconnect
- Full-duplex client (simultaneous send + receive)

### Core Concepts

- Shared state synchronization
- Mutex-protected critical sections
- Concurrent broadcast architecture
- Multi-client group chat behavior

### Behavior

- Any client message is instantly broadcast to all connected clients
- Server supports true concurrent group chat
- Thread-safe client tracking and cleanup

---

## Build & Run

### Compile Server

g++ apexserver.cpp -o ApexServer -lws2_32 -std=c++17

### Compile Client

g++ apexclient.cpp -o ApexClient -lws2_32 -std=c++17

### Run

.\ApexServer.exe  
.\ApexClient.exe

---

## Project Status

Current Version: **v0.4 – Concurrent Broadcast Chat Server**

Next Phase: Controlled concurrency and scalability improvements toward handling 1000+ simultaneous connections.
