#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <queue>

#define PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

class Sender{

public:
    Sender(){

    }

    int initialize(){
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            std::cerr << "Errore nella creazione del socket" << std::endl;
            return 1;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        return 0;
    }

    void main_loop(){
        while(true){
            int n = sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, addr_len);
            if (n < 0) {
                std::cerr << "Errore nell'invio dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }

            n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
            if (n < 0) {
                std::cerr << "Errore nella ricezione dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }

            buffer[n] = '\0';
            std::cout << "Dati inviati al server: " << buffer << std::endl;
            sleep(1);
        }
    }

    int deinitialize(){
        close(sockfd);
        return 0;
    }

    ~Sender(){
        deinitialize();
    }


private:
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    const char *message = "Hello, Server";
    socklen_t addr_len = sizeof(server_addr);

};



int main() {

    Sender s;
    s.initialize();
    s.main_loop();

}