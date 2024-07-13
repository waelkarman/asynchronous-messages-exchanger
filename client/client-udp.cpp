#include <iostream>
#include <functional>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "datapacket.hpp"
#include <queue>
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
 * 
 */

std::mutex task_queue_mutex;
class Sender{

public:
    Sender():seq(0){

    }

    int initialize(){
        // Creazione del socket UDP
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            std::cerr << "Errore nella creazione del socket" << std::endl;
            return 1;
        }

        // Impostazione dell'indirizzo del server
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        return 0;
    }

    void send_data(string data){
        to_send.push(data);
    }

    void main_loop(){
        
        std::function<void()> task0 = [this](){
            this->send_loop();
        };
        std::function<void()> task1 = [this]() {
            this->receive_loop();
        };


        tasks.push_back(std::move(task0));
        tasks.push_back(std::move(task1));

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

    ~Sender(){
        deinitialize();
    }


private:
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    string message = "Hello, Server";
    socklen_t addr_len = sizeof(server_addr);
    size_t seq;
    std::queue<string> to_send;
    std::mutex mtx;
    vector<thread> workers;
    vector<std::function<void()>> tasks;

    void send_loop(){
        while(true){
            // message fetching
            datapacket dp;
            string pack;
            
            if(!to_send.empty()){
                message = to_send.front();
                to_send.pop();
                pack = dp.pack(seq,message);
            }else{
                pack = dp.pack(seq,"NO MESSAGES");
            }
            seq++;
            
            // send data
            int n = sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);
            if (n < 0) {
                std::cerr << "Errore nell'invio dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }

            sleep(1);
        }
    }

    void receive_loop(){
        while(true){
            // get answer from the server

            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
            if (n < 0) {
                std::cerr << "Errore nella ricezione dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }
            buffer[n] = '\0';

            std::cout << "Dati inviati al server: " << buffer << std::endl;
        }
    }

    void th_lodable(vector<std::function<void()>> & tasks){
        while(true){
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

    Sender s;
    s.send_data("CIAOOO");
    s.send_data("CIAOOO");
    s.send_data("CIAOOO");
    s.send_data("CIAOOO");
    s.send_data("CIAOOO");
    s.send_data("CIAOOO");
    s.send_data("CIAOOO");
    s.send_data("CIAOOO");
    s.initialize();
    s.main_loop();
    s.send_data("HALLO");
    s.send_data("HALLO");
    s.send_data("HALLO");
    s.send_data("HALLO");



}
