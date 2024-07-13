#include <iostream>
#include <functional>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <datapacket.hpp>
#include <timer.hpp>
#include <queue>
#include <map>
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
    Sender():key(0),available(0),received(0),milliseconds(1000),missing(false){}

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
        to_send[available]=data;
        q.push(available);
        available++;
    }

    void main_loop(){
        
        std::function<void()> task0 = [this](){
            this->fetch_and_send_loop(this->milliseconds);
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
    bool missing;
    size_t key;
    size_t available;
    size_t received;
    std::queue<size_t> q;
    std::queue<size_t> saved;
    std::map<int,string> to_send;
    std::mutex mtx;
    vector<thread> workers;
    vector<std::function<void()>> tasks;
    int milliseconds;

    void fetch_and_send_loop(int milliseconds){
        while(true){
            // message fetching
            datapacket dp;
            string pack;
            
            if(!q.empty()){
                key = q.front();
                q.pop();

                /**
                 * leggo da una coda e invio
                 * qualcuno riempie la coda e mi dice cosa inviare di nuovo
                 * quello che invio lo metto in waiting list.
                 * 
                 */

                message = to_send[key];
                pack = dp.pack(key,message);
            }else{
                pack = dp.pack(-1,"NO MESSAGES");
            }
            
            // send data
            int n = sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&server_addr, addr_len);
            if (n < 0) {
                std::cerr << "Errore nell'invio dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
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
            string s(buffer);
            size_t rec_key = static_cast<size_t>(std::stoul(s));
            
            if( rec_key >= received){
                if( (rec_key-received) > 0 ){
                    // droppa se il pacchetto è già arrivato 
                    q.push(received);
                    //salva i reg_key da recuperare e recuperali dopo l'arrivo di quella mancante
                    saved.push(rec_key);
                }else{
                    to_send.erase(received);
                    received++;
                    
                    //vedi se c'è qualcosa da recuperare e recupera finchè non se ne perde un altro allora lo rimandi e recuperi dopo il restante.
                    while(!saved.empty() || !missing){
                        if(saved.front() == received){
                            to_send.erase(received);
                            saved.pop();
                            received++;
                        }else{
                            q.push(received);
                            missing=true;
                        }
                    }
                    missing=false;
                }
                std::cout << "Dati inviati al server: " << buffer << std::endl;
            }
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
