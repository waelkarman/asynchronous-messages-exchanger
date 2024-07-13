#include <iostream>
#include <functional>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <datapacket.hpp>
#include "safequeue.hpp"
#include <utils.hpp>
#include "safemap.hpp"
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
    Receiver():key(0),available(0),key_stored(0),broken_pipe(0),milliseconds(1000),missing(false),stop_condition(false){}

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

    void add_to_message_queue(string data, size_t n = 0){
        if(n==0){
            message_queue.insert(available,data);
            query.push(available);
            available++;
        }else{
            message_queue.insert(n,data);
            query.push(n);
        }
    }

    void main_loop(){
        
        std::function<void()> task0 = [this]() {
            this->message_handler_loop();
        };
        std::function<void()> task1 = [this](){
            this->fetch_and_send_loop(this->milliseconds);
        };
        std::function<void()> task2 = [this]() {
            this->acknoledge_handling_loop();
        };
        std::function<void()> task3 = [this]() {
            this->received_message_loop();
        };

        tasks.push_back(std::move(task0));
        tasks.push_back(std::move(task1));
        tasks.push_back(std::move(task2));
        tasks.push_back(std::move(task3));

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
    string message = "Hello, Client";
    socklen_t addr_len = sizeof(client_addr);

    bool missing;
    bool stop_condition;
    int broken_pipe;
    size_t key;
    size_t available;
    size_t key_stored;
    TSQueue<size_t> query;
    TSQueue<size_t> saved;
    TSQueue<size_t> ack_queue;
    TSQueue<string> recv_queue;
    TSMap<size_t,string> message_queue;
    std::mutex mtx;
    vector<thread> workers;
    vector<std::function<void()>> tasks;
    int milliseconds;
    MessageType TYPE_MSG = MSG;
    MessageType TYPE_ACK = ACK;
    datapacket dp;

    void message_handler_loop(){
        while(true){
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            cout << "Ricevuto messaggio da " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << endl;
            if (n < 0) {
                std::cerr << " Errore nella ricezione dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }
            buffer[n] = '\0';
            string s(buffer);
            vector<string> pack = dp.unpack(s);

            switch (stoi(pack[0])) {
                case MSG:
                    cout<<"il messaggio è un MSG"<<endl;
                    recv_queue.push(s);
                    break;
                case ACK:
                    cout<<"il messaggio è un ACK"<<endl;
                    ack_queue.push(static_cast<size_t>(std::stoul(pack[1])));
                    break;
                default:
                    cout<<"messaggio non categorizzato"<<endl;
            }
        }
    }

    void fetch_and_send_loop(int milliseconds){
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        while(!stop_condition){
            // message fetching
            string pack;
            
            if(!query.empty()){
                //printQueue(query);
                query.printQueue();
                key = query.front();
                cout << "send: invio valore estratto " << key << endl;
                // Comment next line to test already received data. 
                query.pop();
                message = message_queue.get(key);
                pack = dp.pack(TYPE_MSG,key,message);
            }else{
                cout << "send: invio valore di default -1" << endl;
                pack = dp.pack(TYPE_MSG,-1,"ALIVE");
            }
            
            // send data 
            int n = sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&client_addr, addr_len);
            if (n < 0) {
                std::cerr << "Errore nell'invio dei dati" << std::endl;
                close(sockfd);
                //return 1;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }
    }

    void acknoledge_handling_loop(){
        while(!stop_condition){

            if(!ack_queue.empty()){
                size_t key_received = ack_queue.front();
                ack_queue.pop();
                
                if( key_received >= key_stored){
                    if( (key_received-key_stored) > 0 && message_queue.find(key_stored)){
                        cout << " chiedo il rinvio di "<< key_stored << endl;

                        query.push(key_stored);
                        cout << " salvo il valore fra i ricevuti non processati "<< key_stored << endl;
                        saved.push(key_received); 
                    }else if(message_queue.find(key_stored)){
                        cout << "ack "<< key_stored <<  " ricevuto, cancello i dati associati " << endl;
                        message_queue.erase(key_stored);
                        
                        while(!saved.empty() && !missing){
                            cout << " Vedi se la lista dei non processati contiene qualcosa" << endl;
                            if(saved.front() == key_stored+1){
                                cout << " RECUPERO il mancante dalla lista" << endl;
                                message_queue.erase(key_stored);
                                saved.pop();
                                key_stored++;
                            }else{
                                cout << " mi fermo a recuperare perche manca "<< key_stored << endl;
                                query.push(key_stored);
                                missing=true;
                            }
                        }
                        key_stored++;
                        missing=false;
                    }
                    
                    //std::cout << " Dati inviati al server: " << buffer << std::endl;

                    if(!message_queue.find(key_stored)&& !message_queue.empty()){
                        cout << " IMPOSSIBILE RECUPERARE I DATI PER IL NUMERO DI SEQUENZA "<< key_stored << endl; 
                        broken_pipe++;
                        if(broken_pipe>2){
                            cout << "fatal error: broken pipe. Transmission failure." << endl;
                            stop_condition=true;
                        }
                    }
                    
                }else{
                    cout<< " DATI ASSICIATO A "<< key_stored << " GIA RICEVUTI"<<endl;
                }

            }
        }
    }

    void received_message_loop(){
        while (true) {
            if(!recv_queue.empty()){
                vector<string> content = dp.unpack(recv_queue.front());
                recv_queue.pop();

                std::cout << "ricevuto  " << content[2] << " invio  ack: "<< content[1] << endl; 
                
                string pack = dp.pack(TYPE_ACK,stoi(content[1]),"ACK_MESSAGE");
                sendto(sockfd, pack.c_str(), strlen(pack.c_str()), 0, (const struct sockaddr *)&client_addr, addr_len);           
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

    void printQueue(const std::queue<size_t>& q) {
        std::queue<size_t> tempQueue = q;
        std::cout << "Message Queue content: ";
        while (!tempQueue.empty()) {
            std::cout << tempQueue.front() << " ";
            tempQueue.pop();
        }
        std::cout << std::endl;
    }
};








int main() {

    Receiver r;
    r.add_to_message_queue("DOG");
    r.initialize();
    r.main_loop();
    r.add_to_message_queue("CAT");
    r.add_to_message_queue("ELEPHANT");
    r.add_to_message_queue("HORSE");
    r.add_to_message_queue("CAMEL");
    r.add_to_message_queue("TURTLE");


}
