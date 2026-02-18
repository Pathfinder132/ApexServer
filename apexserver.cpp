#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> // required header files which contains only declaration
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

//-lws2_32 required to attach when about to compile
#pragma comment(lib, "ws2_32.lib") //LINKER which links the functionality of winsock2 to its header file ( declarations ) and makes it usable
// pragma is a compiler directive which works as LINKER

//ONE TIME THING: WSA startup

/*std::vector<SOCKET> clients;
std::mutex clientsMutex; //Mutex to protect the clients vector from simultaneous access by multiple threads (Race Conditions), its mutual exclusion so both thread1 and thread2 cant write into vector AT SAME TIME.*/

void handleClient(SOCKET clientSocket, int threadID){
    char buffer[1024];
    while(true){
        memset(buffer,0,sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if(bytesReceived<=0){
            std::cout<<"Client "<<threadID<<" Disconnected\n";
            break;
        }
        buffer[bytesReceived]='\0';
        std::cout<<"Client "<<threadID<<": "<<buffer<<std::endl;
        send(clientSocket, buffer,bytesReceived,0);
    }
    closesocket(clientSocket);  
}
int main(){
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(result!=0){
        std::cerr<<"WSAStartup FAILED!\n"; // error output, unbuffered
        return 1;
    }

    //socket creation
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // 0 is for defaulting
    // SOCKET -datatype, socket() OS function to creat new endpoint
    //AF_INET - Address Family Internet (indication to use IPv4)
    //SOCK_STREAM to enable two way communication TCP
    if(serverSocket==INVALID_SOCKET){
        std::cerr<<"Socket creation failed\n";
        WSACleanup(); // always cleanup, ie close the networking system to clean junk
        return 1;
    }

    sockaddr_in serverAddr; // struct structName
    serverAddr.sin_family = AF_INET; // using the struct and setting the network,conversion of htons etc
    serverAddr.sin_port = htons(8000); // converts endian address (least significant like here) to BIG endian ( flips bits ) so the network understands
    serverAddr.sin_addr.s_addr = INADDR_ANY;// automatically get IP address be it ethernet etc

    //BIND socket
    //force convert serverAddr to GENERIC sockaddr even tho its ipv4 using (sockaddr*), size to get the size
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);// closing the failed socket
        WSACleanup();// cleanup
        return 1;
    }

    //LISTEN
    //SOcketMAXimumCONNections - if 100 ppl are trying to acces, they wil be in queue, its an
    //indicator to make queue as large as possible
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    //ACCEPT
    //accept(socket, ipaddress, portaddress) - right now we dont care who we are accepting
    //so we use nullptr's
    while(true){
        static int i=1;
        SOCKET clientSocket = accept(serverSocket, nullptr,nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }
        std::cout << "Client "<<i<<" connected!\n";

        std::thread clientThread(handleClient,clientSocket, i); // creating a thread for the current client, thread accepts (function, 1st arg, 2nd arg, 3rd arg........)
        i++;
        clientThread.detach(); // isolating the newly created thread so that server keeps accepting NEW clients
        
    }


    return 0;
}