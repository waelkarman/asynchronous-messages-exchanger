#include <iostream>
#include <functional>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <datapacket.hpp>

#include <thread>

#define PORT 12345
#define BUFFER_SIZE 1024

/**
 * 
 *  Send n\                 / resend n
 *         \ wait for ack n/
 *          \
 *           \ send n+1
 * 
 *  sinchonizza le strutture dati comuni ai 2 thread
 */

std::mutex task_queue_mutex;
class Receiver{

public:
    Receiver(){}

    int initialize(){
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

        return 0;
    }


    void main_loop(){

        std::function<void()> task0 = [this](){
            this->receive_and_acknoledge_loop();
        };

        tasks.push_back(std::move(task0));

        while(tasks.size()>0){
            workers.push_back(thread([this](vector<std::function<void()>> & tasks){this->th_lodable(tasks);},std::ref(tasks)));
        }
    }


    int deinitialize(){
        for(thread& w : workers){
            if(w.joinable()){
                w.join();
            }
        }
        close(sockfd);
        return 0;
    }

    ~Receiver(){
        deinitialize();
    }

private:
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    vector<thread> workers;
    vector<std::function<void()>> tasks;

    void receive_and_acknoledge_loop(){
        while (true) {
            // Ricezione dei dati dal client
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            if (n < 0) {
                std::cerr << "Errore nella ricezione dei dati" << std::endl;
                continue;
            }

            buffer[n] = '\0';
            std::cout << "SERVER : ricevuto  " << buffer <<endl; 
            std::string s(buffer);
            datapacket dp;
            vector<string> pack = dp.unpack(s);
                    

            // Invia una risposta al client
            const char *response = pack[1].c_str();
            sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&client_addr, addr_len);
        }
    }

    void th_lodable(vector<std::function<void()>> & tasks){
        while(!tasks.empty()){
            std::function<void()> f;
            {
                std::lock_guard<std::mutex> lock(task_queue_mutex);
                
                if (tasks.empty()){
                    return;
                }
                f = tasks.back();
                tasks.pop_back();
                cout << "Thread " << " loaded with task: " << tasks.size() << endl;
            }
            f();
        }
    }

};








int main() {

    Receiver r;
    r.initialize();
    r.main_loop();

}
