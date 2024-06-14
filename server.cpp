#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>

#define PORT 443
#define BUFFER_SIZE 4096

void startListening(int port);

int main() {
    startListening(PORT);
    return 0;
}

void startListening(int port) {
    int listenSocket, clientSocket;
    struct sockaddr_in serverAddr{}, clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);

    // Cria o socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        std::cerr << "Error at socket creation: " << strerror(errno) << std::endl;
        return;
    }

    // Configura o endereço do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Permite reuso do endereço
    int opt = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        close(listenSocket);
        return;
    }

    // Associa o socket ao endereço e porta
    if (bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(listenSocket);
        return;
    }

    // Configura o socket para ouvir por conexões
    if (listen(listenSocket, SOMAXCONN) < 0) {
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(listenSocket);
        return;
    }

    std::cout << "Listening on port " << port << "..." << std::endl;

    // Aceita uma conexão do cliente
    clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientSocket < 0) {
        std::cerr << "Accept failed: " << strerror(errno) << std::endl;
        close(listenSocket);
        return;
    }

    std::cout << "Connection established with client." << std::endl;

    close(listenSocket);

    char buffer[BUFFER_SIZE];
    ssize_t bytesReceived;

    while (true) {
        // Envia comando para o cliente
        std::string command;
        std::cout << "Enter command: ";
        std::getline(std::cin, command);
        command += "\n"; // Adiciona uma nova linha ao comando
        ssize_t bytesSent = send(clientSocket, command.c_str(), command.size(), 0);
        if (bytesSent < 0) {
            std::cerr << "Send failed: " << strerror(errno) << std::endl;
            close(clientSocket);
            return;
        }

        // Limpa o buffer de recepção
        memset(buffer, 0, BUFFER_SIZE);

        // Recebe dados do cliente
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::cout << "Received: " << buffer << std::endl;
        } else if (bytesReceived == 0) {
            std::cout << "Connection closing..." << std::endl;
            break;
        } else {
            std::cerr << "Recv failed: " << strerror(errno) << std::endl;
            close(clientSocket);
            return;
        }
    }

    // Fecha o socket do cliente
    close(clientSocket);
}