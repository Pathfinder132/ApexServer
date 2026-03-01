# ApexServer

A high-performance C++ TCP chat server built using Winsock2 on Windows. This project was developed in structured phases to master low-level socket programming, multi-threaded synchronization, and scalable server architecture.

## Phase 1 – Basic Echo Server

### Features

Single-client TCP server.

Localhost communication (127.0.0.1:8000).

Echo functionality: Verifies end-to-end data transmission.

Manual Winsock initialization (WSAStartup) and cleanup.

### Server Flow

WSAStartup → socket → bind → listen → accept → recv → send → cleanup

## Phase 2 – Persistent Single-Threaded Server

### Improvements

Continuous Loop: Server stays alive after a client disconnects.

Abstraction: Encapsulated client logic into handleClient() functions.

Detection: Implemented proper TCP disconnect handling by checking recv() return values.

### Core Architecture

C++
while (true) {
SOCKET client = accept(serverSocket, nullptr, nullptr);
handleClient(client); // Blocks the main thread until client leaves
}

## Phase 3 – Thread-Per-Client Concurrency

### Improvements

Dynamic Threading: Each connection spawns a new std::thread.

Non-blocking Accept: The main thread remains free to welcome new users immediately.

Introduced OS-level concurrency concepts.

### Core Architecture

C++
while (true) {
SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
std::thread clientThread(handleClient, clientSocket);
clientThread.detach(); // Allow thread to run independently
}

## Phase 4 – Real-Time Broadcast Chat System

### Improvements

Shared State: Implemented a std::vector<SOCKET> to track all active participants.

Synchronization: Used std::mutex to prevent Race Conditions during broadcast loops.

Global Broadcast: Every message received is relayed to every connected socket.

Full-Duplex: Enabled simultaneous sending and receiving.

## Phase 5 – High-Efficiency Thread Pool & Request Queue

This is the final architecture, focusing on resource management and system stability.

### Technical Advancements

Fixed Thread Pool: Pre-allocates 4 Worker Threads to prevent "Thread Exhaustion" and excessive context switching.

Producer-Consumer Pattern: The main thread (Producer) accepts sockets and pushes them to a std::queue (The Waiting Hall), while Workers (Consumers) pull tasks when free.

Efficient Sleeping: Used std::condition_variable to put idle threads to sleep, ensuring 0% CPU usage when no tasks are pending.

Kernel Buffer Draining: Solved "Message Bombardment" by using ioctlsocket with FIONBIO to flush the OS-level buffer before admitting a client from the queue.

### Core Architecture

C++
// Main Thread (The Receptionist/Producer)
while(true) {
SOCKET client = accept(serverSocket, ...);
{
std::lock_guard<std::mutex> lock(queueMutex);
taskQueue.push({client, username});
}
cv.notify_one(); // Wake up one idle worker
}

// Worker Thread (The Consumer)
while(serverRunning) {
std::unique_lock<std::mutex> lock(queueMutex);
cv.wait(lock, []{ return !taskQueue.empty(); }); // Sleep until work arrives

    // Admit to chat vector and enter chat loop...

}

## Build & Run

### Requirements

OS: Windows 10/11

Compiler: MinGW-w64 (g++)

Library: ws2_32 (Windows Sockets)

### Compilation

Bash
g++ apexserver.cpp -o ApexServer -lws2_32 -std=c++17

### Run

Start the server: .\ApexServer.exe

Connect multiple clients: .\ApexClient.exe
