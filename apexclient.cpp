#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main(){
    //WINSOCK startup
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2),&wsaData);
    if(result!=0){
        std::cerr<<"Startup failed!\n";
        return 1;
    }

    //SOCKET creation
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM,0);
    if(clientSocket==INVALID_SOCKET){
        std::cerr<<"Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    //SERVER ADDRESS definition
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8000);

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    //CONNECTION
    if(connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr))==SOCKET_ERROR){
        std::cerr<<"Connection failed\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server!\n";

    //TESTING: SENDING MESSAGE
    const char* message="hello from client!";
    send(clientSocket, message, strlen(message),0);
    
    //RECEIVE data
    char buffer[1024];
    int bytesReceived=recv(clientSocket,buffer,1024,0);

    if (bytesReceived > 0) {
    buffer[bytesReceived] = '\0';  // VERY IMPORTANT
    std::cout << "Server replied: " << buffer << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    
    return 0;
}