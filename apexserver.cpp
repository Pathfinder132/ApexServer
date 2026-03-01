#include <iostream>
#include <winsock2.h> // required header files which contains only declaration
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <string>
#include <queue>
#include <condition_variable>
//-lws2_32 required to attach when about to compile
// Phase 5 is now working on a client-receptionist model where clients are sockets with username and receptionist is the main() thread where it greets and tell the client to either check in to the room or wait in the waiting hall

struct clientJob
{
    SOCKET socket;
    std::string username;
};

std::queue<clientJob> taskQueue;
std::mutex queueMutex;
std::condition_variable cv; // With a condition_variable, the thread "goes to sleep" and uses 0% CPU until the main thread calls cv.notify_one() even if theres while(true).
bool serverRunning = true;

#pragma comment(lib, "ws2_32.lib") // LINKER which links the functionality of winsock2 to its header file ( declarations ) and makes it usable
// pragma is a compiler directive which works as LINKER

// ONE TIME THING: WSA startup

std::vector<SOCKET> clients;
std::mutex clientsMutex; // Mutex to protect the clients vector from simultaneous access by multiple threads (Race Conditions), its mutual exclusion so both thread1 and thread2 cant write into vector AT SAME TIME.

void workerThread(int id)
{
    while (serverRunning == true)
    {
        std::string username;
        SOCKET clientSocket = INVALID_SOCKET;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, []
                    { return !taskQueue.empty() || !serverRunning; });
            clientSocket = taskQueue.front().socket;
            username = taskQueue.front().username;
            // if lambda returns true, it locks and worker thread starts the process, if its false then it unlocks ( free to use by anyone ) and continues sleeping
            // here the work of thread is the rest BELOW CODE
            // cv works even if main thread is slow or fast, cuz mutex exists for locking and unlocking
            taskQueue.pop();
        }
        bool canJoin = false;
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            if (clients.size() < 4)
            {
                canJoin = true;
                clients.push_back(clientSocket);
            }
        }
        if (canJoin)
        {
            std::cout << "[" << username << "]" << " has joined the chat!\n";

            // --- THE FLUSH (One-time fix for bombardment) ---
            std::string welcomeMsg = "Server: You are now out of the waiting hall! You can chat now.\n";
            send(clientSocket, welcomeMsg.c_str(), (int)welcomeMsg.size(), 0);
            u_long mode = 1;
            ioctlsocket(clientSocket, FIONBIO, &mode); // Go non-blocking
            char junk[1024];
            while (recv(clientSocket, junk, sizeof(junk), 0) > 0)
                ; // Eat the old data
            mode = 0;
            ioctlsocket(clientSocket, FIONBIO, &mode); // Go back to normal

            // ------------------------------------------------

            char buffer1[1024];
            while (true)
            {
                memset(buffer1, 0, sizeof(buffer1));                                  // resets or reallocates the buffer
                int bytesReceived1 = recv(clientSocket, buffer1, sizeof(buffer1), 0); // recv() puts data in buffer and returns size of data
                if (bytesReceived1 <= 0)
                {
                    std::cout << "[" << username << "]" << " disconnected\n";
                    {
                        std::lock_guard<std::mutex> lock(clientsMutex); // locking here so that other threads' for loops dont point to garbage or wrong sockets AND other threads CANT erase from clientsvector
                        clients.erase(
                            std::remove(clients.begin(), clients.end(), clientSocket),
                            clients.end());
                    }
                    break;
                }
                buffer1[bytesReceived1] = '\0';
                std::string chatMsg = username + ": " + buffer1; // + operations and to_string operations in <string>

                { // lock here so that all the vector modifications come AFTER send()
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    for (auto client : clients)
                    {
                        send(client, chatMsg.c_str(), (int)chatMsg.size(), 0);
                    }
                }
            }
        }
        else if (!serverRunning)
        {
            closesocket(clientSocket);
            return;
        }
    }
}

int main()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "WSAStartup FAILED!\n"; // error output, unbuffered
        return 1;
    }

    // socket creation
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // 0 is for defaulting
    // SOCKET -datatype/struct/class, socket() OS function to creat new endpoint
    // AF_INET - Address Family Internet (indication to use IPv4)
    // SOCK_STREAM to enable two way communication TCP
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed\n";
        WSACleanup(); // always cleanup, ie close the networking system to clean junk
        return 1;
    }

    sockaddr_in serverAddr;                  // struct structName
    serverAddr.sin_family = AF_INET;         // using the struct and setting the network,conversion of htons etc
    serverAddr.sin_port = htons(8000);       // converts endian address (least significant like here) to BIG endian ( flips bits ) so the network understands
    serverAddr.sin_addr.s_addr = INADDR_ANY; // automatically get IP address be it ethernet etc

    // BIND socket
    // force convert serverAddr to GENERIC sockaddr even tho its ipv4 using (sockaddr*), size to get the size
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket); // closing the failed socket
        WSACleanup();              // cleanup
        return 1;
    }

    // LISTEN
    // SOcketMAXimumCONNections - if 100 ppl are trying to acces, they wil be in queue, its an
    // indicator to make queue as large as possible
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    } // listen is basically doing "im now ready to receive requests"
    //  if not ready then it will give socket error -> return 1

    // ACCEPT
    // accept(socket, ipaddress, portaddress) - right now we dont care who we are accepting
    // so we use nullptr's

    std::vector<std::thread> pool;
    for (int i = 1; i <= 4; ++i)
    {
        pool.emplace_back(workerThread, i);
    }

    while (true)
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET)
        {
            char buffer[1024];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            std::string username = "unknown";
            if (bytesReceived > 0)
            {
                buffer[bytesReceived] = '\0';
                username = std::string(buffer);
            }
            std::string waitMsg = "Server: Hello " + username + ", you are now in the waiting room.........\n";
            send(clientSocket, waitMsg.c_str(), (int)waitMsg.size(), 0);
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                taskQueue.push({clientSocket, username}); // we are pushing a struct with {}
            }
            cv.notify_one(); // Wake up one of the threads you created at the start
        }
    }

    return 0;
}
