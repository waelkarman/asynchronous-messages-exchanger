#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    const char *message = "Hello, Server";
    socklen_t addr_len = sizeof(server_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Errore nella creazione del socket" << std::endl;
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int n = sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, addr_len);
    if (n < 0) {
        std::cerr << "Errore nell'invio dei dati" << std::endl;
        close(sockfd);
        return 1;
    }

    n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
    if (n < 0) {
        std::cerr << "Errore nella ricezione dei dati" << std::endl;
        close(sockfd);
        return 1;
    }

    buffer[n] = '\0';
    std::cout << "Ricevuto dal server: " << buffer << std::endl;

    close(sockfd);
    return 0;
}