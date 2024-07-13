#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <datapacket.hpp>


#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    // Creazione del socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Errore nella creazione del socket" << std::endl;
        return 1;
    }

    // Impostazione dell'indirizzo del server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind del socket all'indirizzo e alla porta specificati
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Errore nel binding del socket" << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "Server in ascolto sulla porta " << PORT << std::endl;

    while (true) {
        // Ricezione dei dati dal client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            std::cerr << "Errore nella ricezione dei dati" << std::endl;
            continue;
        }

        buffer[n] = '\0';
        std::cout << "Ricevuto: " << buffer << " da " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;

        std::string s(buffer);
        datapacket dp;
        vector<string> pack = dp.unpack(s);
                

        // Invia una risposta al client
        const char *response = pack[0].c_str();
        sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
